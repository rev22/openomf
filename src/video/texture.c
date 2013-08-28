#include "video/texture.h"
#include "utils/log.h"
#include <stdlib.h>
#include <memory.h>

int texture_create(texture *tex, const char *data, unsigned int w, unsigned int h) {
    tex->w = tex->bitmap_w = w;
    tex->h = tex->bitmap_h = h;
    
    tex->adapted = 0;
    
    // If data is null, then we create an uninitialized texture
    // Otherwise, copy the raw image data to a buffer.
    tex->data = NULL;
    if(data != NULL) {
        tex->data = malloc(w * h * 4);
        memcpy(tex->data, data, w * h * 4);
    }
    return 1;
}

int texture_create_from_img(texture *tex, const image *img) {
    return texture_create(tex, img->data, img->w, img->h);
}

void texture_free(texture *tex) {
    if(tex->data != 0) {
        free(tex->data);
        tex->data = NULL;
        //glDeleteTextures(1, &tex->id);
        tex->w = 0;
        tex->h = 0;
    }
}

unsigned int texture_size(texture *tex) {
    if(tex->data == 0) {
        return 0;
    }
    return tex->bitmap_w * tex->bitmap_h * 4;
}

int texture_pix_opaque(texture *tex, unsigned int x, unsigned int y) {
    return (tex->data[(x + y * tex->bitmap_w) * 4 + 3] > 0);
}
