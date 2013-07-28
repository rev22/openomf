#include "controller/keyboard.h"
#include "utils/log.h"

void keyboard_free(controller *ctrl) {
    keyboard *k = ctrl->data;
    free(k->keys);
    free(k);
}

int keyboard_tick(controller *ctrl) {
    keyboard *k = ctrl->data;
    const unsigned char *state = SDL_GetKeyboardState(NULL);
    int handled = 0;

    if (state[k->keys->down]) {
        controller_cmd(ctrl, ACT_CROUCH);
        handled = 1;
    }
    if (state[k->keys->up]) {
        controller_cmd(ctrl, ACT_JUMP);
        handled = 1;
    }
    if(state[k->keys->right] && !state[k->keys->left] &&
            !state[k->keys->down] && !state[k->keys->kick] && !state[k->keys->punch]) {
        controller_cmd(ctrl, ACT_WALKRIGHT);
        handled = 1;
    }
    if(state[k->keys->left] && !state[k->keys->right] &&
            !state[k->keys->down] && !state[k->keys->kick] && !state[k->keys->punch]) {
        controller_cmd(ctrl, ACT_WALKLEFT);
        handled = 1;
    }
    if(!handled) {
        controller_cmd(ctrl, ACT_STOP);
    }
    return 0;
}

int keyboard_handle(controller *ctrl, SDL_Event *event) {
    keyboard *k = ctrl->data;
    if (event->type == SDL_KEYUP && (event->key.keysym.sym == k->keys->kick || event->key.keysym.sym == k->keys->punch)) {
        return 1;
    }
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        const unsigned char *state = SDL_GetKeyboardState(NULL);
        if ( state[k->keys->left] && state[k->keys->up]) {
            controller_cmd(ctrl, ACT_UPLEFT);
        } else if ( state[k->keys->left] && state[k->keys->down]) {
            controller_cmd(ctrl, ACT_DOWNLEFT);
        } else  if ( state[k->keys->right] && state[k->keys->up]) {
            controller_cmd(ctrl, ACT_UPRIGHT);
        } else  if ( state[k->keys->right] && state[k->keys->down]) {
            controller_cmd(ctrl, ACT_DOWNRIGHT);
        } else if ( state[k->keys->right]) {
            controller_cmd(ctrl, ACT_RIGHT);
        } else if ( state[k->keys->left]) {
            controller_cmd(ctrl, ACT_LEFT);
        } else if ( state[k->keys->up]) {
            controller_cmd(ctrl, ACT_UP);
        } else if ( state[k->keys->down]) {
            controller_cmd(ctrl, ACT_DOWN);
        }

        if (state[k->keys->punch]) {
            controller_cmd(ctrl, ACT_PUNCH);
        } else if (state[k->keys->kick]) {
            controller_cmd(ctrl, ACT_KICK);
        }
        return 0;
    }
    // If key was handled here, return 0. Otherwise 1.
    return 1;
}

void keyboard_create(controller *ctrl, keyboard_keys *keys) {
    keyboard *k = malloc(sizeof(keyboard));
    k->keys = keys;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_KEYBOARD;
    ctrl->tick_fun = &keyboard_tick;
    ctrl->handle_fun = &keyboard_handle;
    /*controller_add_hook(ctrl, &hook);*/
}
