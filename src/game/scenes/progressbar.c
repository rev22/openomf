#include "game/scenes/progressbar.h"
#include "video/image.h"
#include "video/video.h"

void progressbar_create_block(progress_bar *bar) {
    float prog = bar->percentage / 100.0f;
    int w = bar->w * prog;
    if(w > 0) {
        image tmp;
        image_create(&tmp, w, bar->h);
        image_clear(&tmp, bar->int_bg_color);
        image_rect_bevel(&tmp, 
                         0, 0, w - 1, bar->h-1, 
                         bar->int_topleft_color, 
                         bar->int_bottomright_color, 
                         bar->int_bottomright_color, 
                         bar->int_topleft_color);
        texture_create_from_img(&bar->block, &tmp);
        image_free(&tmp);
    }
}

void progressbar_create(progress_bar *bar,     
                       unsigned int x, unsigned int y,
                       unsigned int w, unsigned int h,
                       color border_topleft_color,
                       color border_bottomright_color,
                       color bg_color,
                       color int_topleft_color,
                       color int_bottomright_color,
                       color int_bg_color,
                       int orientation) {
    bar->x = x;
    bar->y = y;
    bar->w = w;
    bar->h = h;
    bar->orientation = orientation;
    bar->percentage = 100;
    bar->int_topleft_color = int_topleft_color;
    bar->int_bottomright_color = int_bottomright_color;
    bar->int_bg_color = int_bg_color;
    
    // Temporary image for drawing necessary textures
    image tmp;
    
    // Background
    image_create(&tmp, w, h);
    image_clear(&tmp, bg_color);
    image_rect_bevel(&tmp, 
                     0, 0, w-1, h-1, 
                     border_topleft_color, 
                     border_bottomright_color, 
                     border_bottomright_color, 
                     border_topleft_color);
    texture_create_from_img(&bar->background, &tmp);
    image_free(&tmp);
    
    // Bar
    progressbar_create_block(bar);
}

void progressbar_free(progress_bar *bar) {
    texture_free(&bar->background);
    texture_free(&bar->block);
}

void progressbar_set(progress_bar *bar, unsigned int percentage) {
    bar->percentage = (percentage > 100 ? 100 : percentage);
    texture_free(&bar->block);
    progressbar_create_block(bar);
}

void progressbar_render(progress_bar *bar) {
    float prog = bar->percentage / 100.0f;
    int rw = bar->w - bar->w * prog;
    video_render_sprite(&bar->background, bar->x, bar->y, BLEND_ALPHA_FULL);
    if(prog > 0) {
        video_render_sprite(&bar->block, bar->x + (bar->orientation == PROGRESSBAR_LEFT ? 0 : rw + 1), bar->y, BLEND_ALPHA_FULL);
    }
}
