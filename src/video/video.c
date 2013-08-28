// Copyright 2013 Tuomas Virtanen
// Copyright 2013 Michele Bini <michele.bini@gmail.com>

// This file is part of OpenOMF
// OpenOMF is developed under the MIT License. Please read LICENSE for more information.

#include "video/texture.h"
#include "video/video.h"
#include "video/glextloader.h"
#include "video/shaderprogram.h"
#include "video/shader.h"
#include "video/image.h"
#include "utils/log.h"
#include "utils/list.h"
#include <SDL/SDL.h>
#include <stdlib.h>

SDL_Surface *screen;
int screen_w, screen_h;

int video_init(int window_w, int window_h, int fullscreen, int vsync) {
    screen_w = window_w;
    screen_h = window_h;

    // Open window
    screen = SDL_SetVideoMode(
        window_w,
        window_h,
	32,
        SDL_SWSURFACE|(fullscreen?SDL_FULLSCREEN:0)
    );
    if ( screen == NULL ) {
        PERROR("Could not set video %dx%d mode: %s", window_w, window_h, SDL_GetError());
        return 1;
    }
        
    // Show some info
    DEBUG("Video Init OK");
    return 0;
}

int video_reinit(int window_w, int window_h, int fullscreen, int vsync) {
    return 0;
}

void video_screenshot(image *img) {
    image_create(img, screen_w, screen_h);
    // glReadBuffer(GL_FRONT);
    // glReadPixels(0, 0, img->w, img->h, GL_BGR, GL_UNSIGNED_BYTE, img->data);
}

void video_render_prepare() {
    SDL_LockSurface(screen);
}

static void adapt_texture(texture*tex) {
  // Convert texture data to screen color-component order
  if (screen == NULL) {
    fprintf(stderr, "Internal error: screen not yet initialized!");
    exit(EXIT_FAILURE);
  }
  Uint32 *d = (void*)(tex->data);
  if (d) {
    Uint8 Rshift = screen->format->Rshift;
    Uint8 Gshift = screen->format->Gshift;
    Uint8 Bshift = screen->format->Bshift;
    int c;
    Uint32 s = tex->w * tex->h;
    for (c = 0; c < s; c++) {
      Uint32 v = d[c];
      v = ((v&0xff)<<Rshift)|(((v>>8)&0xff)<<Gshift)|(((v>>16)&0xff)<<Bshift)|(v&0xff000000);
      d[c] = v;
    }
    tex->adapted = 1;
  }
}

void video_render_background(texture *tex) {
    video_render_sprite(tex,0,0,BLEND_ALPHA);
}

void video_render_char(texture *tex, int sx, int sy, unsigned char r, unsigned char g, unsigned char b) {
    Uint8 Rshift = screen->format->Rshift;
    Uint8 Gshift = screen->format->Gshift;
    Uint8 Bshift = screen->format->Bshift;
    Uint32 color = (r<<Rshift)|(g<<Gshift)|(b<<Bshift);

    unsigned int sw = screen->w;
    unsigned int sh = screen->h;
    unsigned int w = tex->w;
    unsigned int h = tex->h;

    int y_0, y_m, y_l;
    if (sy < 0) {
      y_0 = -sy;
      sy = 0;
    } else {
      y_0 = 0;
    }
    y_l = sh - sy;
    {
      int y_l1 = h - y_0;
      if (y_l < y_l1) {
	y_m = y_0 + y_l;
      } else {
	y_m = h;
	y_l = y_l1;
      }
    }
    if (y_l < 0) return;

    int x_0, x_m, x_l;
    if (sx < 0) {
      x_0 = -sx;
      sx = 0;
    } else {
      x_0 = 0;
    }
    x_l = sw - sx;
    {
      int x_l1 = w - x_0;
      if (x_l < x_l1) {
	x_m = x_0 + x_l;
      } else {
	x_m = w;
	x_l = x_l1;
      }
    }
    if (x_l < 0) return;
    
    unsigned int l = SDL_MUSTLOCK(screen);
    if (l) SDL_LockSurface(screen);

    Uint32 *sd = screen->pixels;
    unsigned int sp = screen->pitch;
    Uint32 *d = (void*)(tex->data);

    int x;
    int y;

    sp /= sizeof(Uint32);

    for (y = y_0; y < y_m; y++, sy++) {
      for (x = x_0; x < x_m; x++, sx++) {
	Uint32 v = d[y*w+x];
	if (v&0xff000000) {
	  sd[sy*sp + sx] = color;
	}
      }
      sx -= x_l;
    }
    
    if (l) SDL_UnlockSurface(screen);
}

void video_render_sprite(texture *tex, int sx, int sy, unsigned int rendering_mode) {
    video_render_sprite_flip(tex, sx, sy, rendering_mode, FLIP_NONE);
}

static inline Uint8 blend_a(Uint8 o, Uint8 v, Uint8 alpha) {
  Uint32 r = (((v*alpha)>>6) + o);
  return ((r>>8)?0xff:r);
}

// sd[si] = (v*alpha + o*anti)/0xff;
// sd[si] = (v*alpha + (0xff-alpha)*o)/0xff;
// sd[si] = (v*alpha + 0xff*o - alpha*o)/0xff;
// sd[si] = (v*alpha - alpha*o)/0xff + o;
// sd[si] = (v - o)*alpha/0xff + o;
#define BLEND(o, v, alpha) ((((v - o)*alpha)>>8) + o)
#define BLENDA(o, v, alpha) blend_a(o, v, alpha)

void video_render_sprite_flip(texture *tex, int sx, int sy, unsigned int rendering_mode, unsigned int flip_mode) {
    if (!tex->adapted) adapt_texture(tex);

    unsigned int sw = screen->w;
    unsigned int sh = screen->h;
    unsigned int w = tex->w;
    unsigned int h = tex->h;

    int y_0, y_m, y_l;
    if (sy < 0) {
      y_0 = -sy;
      sy = 0;
    } else {
      y_0 = 0;
    }
    y_l = sh - sy;
    {
      int y_l1 = h - y_0;
      if (y_l < y_l1) {
	y_m = y_0 + y_l;
      } else {
	y_m = h;
	y_l = y_l1;
      }
    }
    if (y_l < 0) return;

    int x_0, x_m, x_l;
    if (sx < 0) {
      x_0 = -sx;
      sx = 0;
    } else {
      x_0 = 0;
    }
    x_l = sw - sx;
    {
      int x_l1 = w - x_0;
      if (x_l < x_l1) {
	x_m = x_0 + x_l;
      } else {
	x_m = w;
	x_l = x_l1;
      }
    }
    if (x_l < 0) return;
    
    unsigned int l = SDL_MUSTLOCK(screen);
    if (l) SDL_LockSurface(screen);

    Uint32 *sd = screen->pixels;
    unsigned int sp = screen->pitch;
    Uint32 *d = (void*)(tex->data);

    int x;
    int y;

    sp /= sizeof(Uint32);

    if (rendering_mode == BLEND_ALPHA) {
      if (flip_mode&FLIP_VERTICAL) {
	if (flip_mode&FLIP_HORIZONTAL) {
	  for (y = y_m - 1; y >= y_0; y--, sy++) {
	    for (x = x_m - 1; x >= x_0; x--, sx++) {
	      Uint32 v = d[y*w+x];
	      if (v&0xff000000) {
		sd[sy*sp + sx] = v&0xffffff;
	      }
	    }
	    sx -= x_l;
	  }
	} else {
	  for (y = y_m - 1; y >= y_0; y--, sy++) {
	    for (x = x_0; x < x_m; x++, sx++) {
	      Uint32 v = d[y*w+x];
	      if (v&0xff000000) {
		sd[sy*sp + sx] = v&0xffffff;
	      }
	    }
	    sx -= x_l;
	  }
	}
      } else {
	if (flip_mode&FLIP_HORIZONTAL) {
	  for (y = y_0; y < y_m; y++, sy++) {
	    for (x = x_m - 1; x >= x_0; x--, sx++) {
	      Uint32 v = d[y*w + x];
	      if (v&0xff000000) {
		sd[sy*sp + sx] = v&0xffffff;
	      }
	    }
	    sx -= x_l;
	  }
	} else {
	  for (y = y_0; y < y_m; y++, sy++) {
	    for (x = x_0; x < x_m; x++, sx++) {
	      Uint32 v = d[y*w + x];
	      if (v&0xff000000) {
		sd[sy*sp + sx] = v&0xffffff;
	      }
	    }
	    sx -= x_l;
	  }
	}
      }
    } else if (rendering_mode == BLEND_ALPHA_FULL) {
      if (flip_mode&FLIP_VERTICAL) {
	if (flip_mode&FLIP_HORIZONTAL) {
	  for (y = y_m - 1; y >= y_0; y--, sy++) {
	    for (x = x_m - 1; x >= x_0; x--, sx++) {
	      Uint32 si = sy*sp + sx;
	      Uint32 v = d[y*w+x];
	      Uint32 o = sd[si];
	      Uint8 alpha = v>>24;
	      sd[si] =
		BLEND((o&0xff), (v&0xff), alpha) |
		BLEND(((o>>8)&0xff), ((v>>8)&0xff), alpha) << 8 |
		BLEND(((o>>16)&0xff), ((v>>16)&0xff), alpha) << 16;
	    }
	    sx -= x_l;
	  }
	} else {
	  for (y = y_m - 1; y >= y_0; y--, sy++) {
	    for (x = x_0; x < x_m; x++, sx++) {
	      Uint32 si = sy*sp + sx;
	      Uint32 v = d[y*w+x];
	      Uint32 o = sd[si];
	      Uint8 alpha = v>>24;
	      sd[si] =
		BLEND((o&0xff), (v&0xff), alpha) |
		BLEND(((o>>8)&0xff), ((v>>8)&0xff), alpha)<<8 |
		BLEND(((o>>16)&0xff), ((v>>16)&0xff), alpha) << 16;
	    }
	    sx -= x_l;
	  }
	}
      } else {
	if (flip_mode&FLIP_HORIZONTAL) {
	  for (y = y_0; y < y_m; y++, sy++) {
	    for (x = x_m - 1; x >= x_0; x--, sx++) {
	      Uint32 si = sy*sp + sx;
	      Uint32 v = d[y*w+x];
	      Uint32 o = sd[si];
	      Uint8 alpha = v>>24;
	      sd[si] =
		BLEND((o&0xff), (v&0xff), alpha) |
		BLEND(((o>>8)&0xff), ((v>>8)&0xff), alpha)<<8 |
		BLEND(((o>>16)&0xff), ((v>>16)&0xff), alpha) << 16;
	    }
	    sx -= x_l;
	  }
	} else {
	  for (y = y_0; y < y_m; y++, sy++) {
	    for (x = x_0; x < x_m; x++, sx++) {
	      Uint32 si = sy*sp + sx;
	      Uint32 v = d[y*w+x];
	      Uint32 o = sd[si];
	      Uint8 alpha = v>>24;
	      sd[si] =
		BLEND((o&0xff), (v&0xff), alpha) |
		BLEND(((o>>8)&0xff), ((v>>8)&0xff), alpha)<<8 |
		BLEND(((o>>16)&0xff), ((v>>16)&0xff), alpha) << 16;
	    }
	    sx -= x_l;
	  }
	}
      }
    } else if (rendering_mode == BLEND_ADDITIVE) {
      if (flip_mode&FLIP_VERTICAL) {
	if (flip_mode&FLIP_HORIZONTAL) {
	  for (y = y_m - 1; y >= y_0; y--, sy++) {
	    for (x = x_m - 1; x >= x_0; x--, sx++) {
	      Uint32 si = sy*sp + sx;
	      Uint32 v = d[y*w+x];
	      Uint32 o = sd[si];
	      Uint8 alpha = v>>24;
	      sd[si] =
		BLENDA((o&0xff), (v&0xff), alpha) |
		BLENDA(((o>>8)&0xff), ((v>>8)&0xff), alpha)<<8 |
		BLENDA(((o>>16)&0xff), ((v>>16)&0xff), alpha) << 16;
	    }
	    sx -= x_l;
	  }
	} else {
	  for (y = y_m - 1; y >= y_0; y--, sy++) {
	    for (x = x_0; x < x_m; x++, sx++) {
	      Uint32 si = sy*sp + sx;
	      Uint32 v = d[y*w+x];
	      Uint32 o = sd[si];
	      Uint8 alpha = v>>24;
	      sd[si] =
		BLENDA((o&0xff), (v&0xff), alpha) |
		BLENDA(((o>>8)&0xff), ((v>>8)&0xff), alpha)<<8 |
		BLENDA(((o>>16)&0xff), ((v>>16)&0xff), alpha) << 16;
	    }
	    sx -= x_l;
	  }
	}
      } else {
	if (flip_mode&FLIP_HORIZONTAL) {
	  for (y = y_0; y < y_m; y++, sy++) {
	    for (x = x_m - 1; x >= x_0; x--, sx++) {
	      Uint32 si = sy*sp + sx;
	      Uint32 v = d[y*w+x];
	      Uint32 o = sd[si];
	      Uint8 alpha = v>>24;
	      sd[si] =
		BLENDA((o&0xff), (v&0xff), alpha) |
		BLENDA(((o>>8)&0xff), ((v>>8)&0xff), alpha)<<8 |
		BLENDA(((o>>16)&0xff), ((v>>16)&0xff), alpha) << 16;
	    }
	    sx -= x_l;
	  }
	} else {
	  for (y = y_0; y < y_m; y++, sy++) {
	    for (x = x_0; x < x_m; x++, sx++) {
	      Uint32 si = sy*sp + sx;
	      Uint32 v = d[y*w+x];
	      Uint32 o = sd[si];
	      Uint8 alpha = v>>24;
	      sd[si] =
		BLENDA((o&0xff), (v&0xff), alpha) |
		BLENDA(((o>>8)&0xff), ((v>>8)&0xff), alpha)<<8 |
		BLENDA(((o>>16)&0xff), ((v>>16)&0xff), alpha) << 16;
	    }
	    sx -= x_l;
	  }
	}
      }      
    }
    
    if (l) SDL_UnlockSurface(screen);
}


void video_render_finish() {
    SDL_UnlockSurface(screen);
    // Flip screen buffer
    SDL_Flip(screen);
}

void video_close() {
    SDL_Quit();
}
