#ifndef _ANIMATION_H
#define _ANIMATION_H

#include "utils/array.h"
#include <shadowdive/shadowdive.h>

typedef struct animation_t {
    sd_animation *sdani;
    array sprites;
    char *soundtable;
} animation;

int animation_create(animation *ani, sd_animation *sdani, sd_palette *pal, int overlay, char *soundtable);
void animation_free(animation *ani);

#endif // _ANIMATION_H
