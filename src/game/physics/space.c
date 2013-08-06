#include "game/physics/space.h"
#include "game/physics/intersect.h"
#include "utils/log.h"
#include <stdlib.h>

physics_space *global_space = NULL;

void physics_space_init() {
    global_space = malloc(sizeof(physics_space));
    vector_create(&global_space->objects, sizeof(object*));
}

vec2i vec2f_to_i(vec2f f) {
    vec2i i;
    i.x = f.x;
    i.y = f.y;
    return i;
}

void physics_space_tick() {
    iterator it;
    object *a, *b, **t;
    unsigned int size;

    // Collisions:
    // - Happen only for objects that are marked to be on same layers (bitmask obj->layers)
    // - Happen only for objects that are in different groups (objects in same group do not interact)
    
    // If collision happens:
    // Soft collision = Bounding box collision, call user defined callback (if not NULL)
    // Hard collision = Stop object or bounce it off (depending on what has been defined)
    
    // Intersections can be checked with shape_intersect. 
    
    // We should also make "inside-out" box, that checks that object is inside the box and
    // not outside (or something) for making sure the hars do not go outside the map.
    // Or maybe just make the walls out of rectangles ? Or something.
    
    // Handle movement
    vector_iter_begin(&global_space->objects, &it);
    while((t = iter_next(&it)) != NULL) {
        a = *t;
        if(!a->is_static) {
            a->pos.x += a->vel.x;
            a->pos.y += a->vel.y;
            a->vel.y += a->gravity;
        }
    }
    
    // Check for collisions
    size = vector_size(&global_space->objects);
    for(int i = 0; i < size; i++) {
        a = *((object**)vector_get(&global_space->objects, i));
        for(int k = i+1; k < size; k++) {
            b = *((object**)vector_get(&global_space->objects, k));
            if((a->group != b->group || a->group == OBJECT_NO_GROUP || b->group == OBJECT_NO_GROUP) && a->layers & b->layers) {
                if(a->col_shape_hard != NULL && b->col_shape_hard != NULL) {
                    if(shape_intersect(a->col_shape_hard, vec2f_to_i(a->pos), b->col_shape_hard, vec2f_to_i(b->pos))) {
                        // For now, just zero out the velocity of colliding objects. We also need to take into account static objects.
                        object_set_vel(a, 0.0f, 0.0f);
                        object_set_vel(b, 0.0f, 0.0f); 
                    }
                }
                if(a->col_shape_soft != NULL && b->col_shape_soft != NULL) {
                    if(shape_intersect(a->col_shape_soft, vec2f_to_i(a->pos), b->col_shape_soft, vec2f_to_i(b->pos))) {
                        DEBUG("Soft collision!");
                        a->ev_collision(a,b,a->userdata,b->userdata);
                    }
                }
            }
        }
    }
}

void physics_space_add(object *obj) {
    vector_append(&global_space->objects, &obj);
}

void physics_space_remove(object *obj) {
    iterator it;
    object **o;
    vector_iter_begin(&global_space->objects, &it);
    while((o = iter_next(&it)) != NULL) {
        if(*o == obj) {
            vector_delete(&global_space->objects, &it);
        }
    }
}

void physics_space_close() {
    vector_free(&global_space->objects);
    free(global_space);
    global_space = NULL;
}