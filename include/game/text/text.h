#ifndef _TEXT_H
#define _TEXT_H

#include "utils/vector.h"
#include "video/texture.h"

enum {
    FONT_UNDEFINED,
    FONT_BIG,
    FONT_SMALL
};

typedef struct font_t font;

struct font_t {
    int size;
    int w,h;
    vector textures;
};

// globals, yay
extern font font_small;
extern font font_large;

int fonts_init();
void fonts_close();
void font_render(font *font, const char *text, int x, int y, unsigned char r, unsigned char g, unsigned char b);
void font_render_wrapped(font *font, const char *text, int x, int y, int w, unsigned char r, unsigned char g, unsigned char b);

#endif // _TEXT_H
