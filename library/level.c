#include "body.h"
#include "list.h"
#include "scene.h"
#include "level.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mystring.h"

const int MAX_LINE_LENGTH = 100;
const int MAX_PROJ_PER_SHIP = 4;

/**
 * A generalization of scene to .
 * The level automatically resizes to store
 * arbitrarily many dynamic and force creators, but has a static
 * number of background objects, and a finite list of 
 * walls.
 */
typedef struct level {
    scene_t *scene;
    list_t *walls;
    list_t *dynamic_objs;
} level_t;


scene_t *level_scene(level_t *level) {
    return level->scene;
}

void free_traj(trajectory_t *traj) {
    list_free(traj->positions);
    free(traj);
}

body_t *get_bodies_from_array(strarray *arr) {
    list_t *vector_list = list_init(3, (void *)free);
    vector_t *point1 = malloc(sizeof(vector_t));
    *point1 = (vector_t) {strtod(arr -> data[0], NULL), strtod(arr ->data[1], NULL)};
    list_add(vector_list, (void *) point1);
    vector_t *point2 = malloc(sizeof(vector_t));
    *point2 = (vector_t) {strtod(arr -> data[2], NULL), strtod(arr ->data[3], NULL)};
    list_add(vector_list, (void *) point2);
    vector_t *point3 = malloc(sizeof(vector_t));
    *point3 = (vector_t) {strtod(arr -> data[4], NULL), strtod(arr ->data[5], NULL)};
    list_add(vector_list, (void *) point3);
    body_t *body1 = body_init(vector_list, 20, (rgb_color_t){0, 1, 1});
    return body1;
}

strarray *get_split_line_from_file(FILE *f) {
    char *line = malloc((MAX_LINE_LENGTH + 1) * sizeof(char));
    char *letter = malloc(sizeof(char));
    fread(letter, 1, 1, f);
    size_t i;
    for (i = 0; *letter != '\n'; i++) {
        line[i] = *letter;
        fread(letter, 1, 1, f);
        *letter = *letter;
    }
    size_t j;
    for (j = i; j < MAX_LINE_LENGTH; j++) {
        line[j] = ' ';
    }
    line[j] = '\0';
    strarray *split = strsplit(line);
    free(line);
    free(letter);
    return split;
}

level_t *level_init_from_folder(char *path) {
    level_t *level = malloc(sizeof(level));
    char *wallpath = malloc((strlen(path) + 11) * sizeof(char));
    //wallpath = strcat(path, "/walls.dat");
    strcat(wallpath, path);
    strcat(wallpath, "/walls.dat");
    FILE *wall_file = fopen(wallpath, "r");
    assert(wall_file != NULL);
    char *enemypath = malloc((strlen(path) + 11) * sizeof(char));
    //enemypath = strcat(path, "/enemy.dat");
    strcat(enemypath, path);
    strcat(enemypath, "/enemy.dat");
    FILE *enemy_file = fopen(enemypath, "r");
    assert(enemy_file != NULL);
    strarray *info = get_split_line_from_file(wall_file);
    size_t num_walls = atoi(info->data[0]);
    level->walls = list_init(num_walls, NULL);
    free_strarray(info);
    info = get_split_line_from_file(enemy_file);
    size_t num_ships = atoi(info->data[0]) + 1;
    size_t num_dynamic = num_ships * (1 + MAX_PROJ_PER_SHIP);
    level->dynamic_objs = list_init(num_dynamic, NULL);
    free_strarray(info);
    level->scene = scene_init_fixed_size(num_walls + num_dynamic,
                                 num_dynamic * (num_walls + num_dynamic));
                                 
    for (size_t i = 0; i < num_walls; i++) {
        info = get_split_line_from_file(wall_file);
        body_t *new_body = get_bodies_from_array(info);
        scene_add_body(level->scene, new_body);
        list_add(level->walls, (void *) new_body);
        free_strarray(info);
    }
    for (size_t i = 0; i < num_ships - 1; i++) {
        info = get_split_line_from_file(enemy_file);
        body_t *new_body = get_bodies_from_array(info);
        scene_add_body(level->scene, new_body);
        list_add(level->dynamic_objs, (void *) new_body);
        free_strarray(info);
    }
    //TODO: add our ship
    return level;
}

void level_free(level_t *level) {
    list_free(level->walls);
    list_free(level->dynamic_objs);
    scene_free(level->scene);
}

size_t level_bodies(level_t *level) {
    return scene_bodies(level->scene);
}

void level_add_dynamic_body(level_t *level, body_t *body) {
    list_add(level->dynamic_objs, body);
    scene_add_body(level->scene, body);
}

list_t *level_predict(level_t *level, list_t *extras, size_t nsteps, double dt) {
    // This might be WAY WAY too slow to be practical, but I have hope
    return NULL;
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

