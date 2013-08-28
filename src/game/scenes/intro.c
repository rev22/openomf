#include "game/scene.h"
#include "game/scenes/intro.h"
#include <SDL/SDL.h>
#include <shadowdive/shadowdive.h>

int intro_init(scene *scene) {
    return 0;
}

void intro_deinit(scene *scene) {

}

void intro_tick(scene *scene) {

}

int intro_event(scene *scene, SDL_Event *e) {
    switch(e->type) {
    case SDL_KEYDOWN:
        if(e->key.keysym.sym == SDLK_ESCAPE) {
            scene->next_id = SCENE_MENU;
            return 1;
        }
        break;
    }
    return 1;
}

void intro_render(scene *scene) {

}

void intro_load(scene *scene) {
    scene->event = intro_event;
    scene->render = intro_render;
    scene->init = intro_init;
    scene->deinit = intro_deinit;
    scene->tick = intro_tick;
}
