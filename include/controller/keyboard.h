#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "controller/controller.h"
#include <SDL/SDL.h>

typedef struct keyboard_keys_t keyboard_keys;
typedef struct keyboard_t keyboard;

struct keyboard_keys_t {
    int up;
    int down;
    int left;
    int right;
    int punch;
    int kick;
};

struct keyboard_t {
    keyboard_keys *keys;
};

void keyboard_create(controller *ctrl, keyboard_keys *keys);
void keyboard_free(controller *ctrl);

#endif // _KEYBOARD_H
