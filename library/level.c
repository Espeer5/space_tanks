#include "body.h"
#include "list.h"
#include "scene.h"
#include "level.h"

/**
 * A generalization of scene to .
 * The level automatically resizes to store
 * arbitrarily many dynamic and force creators, but has a static
 * number of background objects, and a finite list of 
 * walls.
 */
typedef struct level {
    scene_t *scene;
    scene_t *prediction_scene;
    list_t *walls;
    list_t *dynamic_objs;
} level_t;

void free_traj(trajectory_t *traj) {
    free_list(traj->positions);
    free(traj);
}

void add_bodies_from_file(char *filename) {

}

scene_t *level_init_from_folder(char *path) {
    level_t *level = malloc(sizeof(level));
    //level->scene = scene_init_fixed_size(?, ?);
    //level->prediction_scene= ??
    //level->walls = list_init(, NULL)
    //level->dynamic_objs = list_init(, NULL);
    // TODO
    // We need to make sure we allocate enough space for dynamic objects
    // such that we never need to resize. Knowledge of the game should
    // be sufficient for this, as long as we read the correct number of 
    // objects in.
    // It is also important to give NULL freers, since these bodies
    // belong actually to the scene, instead of these lists.
}

void level_free(level_t *level) {
    list_free(level->walls);
    list_free(level->dynamic_objs);
    scene_free(level->scene);
}

size_t level_bodies(level_t *level) {
    return scene_bodies(level->scene);
}

void scene_add_dynamic_body(level_t *level, body_t *body) {
    list_add(level->dynamic_objs, body);
    scene_add_body(level->scene, body);
}

list_t *level_predict(level_t *level, list_t *extras, size_t nsteps, double dt) {
    // This might be WAY WAY too slow to be practical, but I have hope

}

void level_tick(level_t *level, double dt) {
    scene_tick_forces(level->scene);
    for (size_t i = list_size(level->dynamic_objs) - 1; i >= 0; i--) {
        if (body_is_removed((body_t *) list_get(level->dynamic_objs, i))) {
            list_remove(level->dynamic_objs, i);
        }
    }
    for (size_t i = list_size(level->walls) - 1; i >= 0; i--) {
        if (body_is_removed((body_t *) list_get(level->walls, i))) {
            list_remove(level->walls, i);
        }
    }
    scene_tick_after_forces(level->scene, dt);
}

