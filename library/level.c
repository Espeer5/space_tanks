#include "body.h"
#include "list.h"
#include "scene.h"
#include "level.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mystring.h"
#include "utils.h"
#include <math.h>

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
    list_t *rocks;
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

// level_t *get_asteroid_info(char *path) {
//     // level_t *level = malloc(sizeof(level));
//     char *rockspath = malloc((strlen(path) + 11) * sizeof(char));
//     strcat(rockspath, path);
//     strcat(rockspath, "/rocks.dat");
//     FILE *rocks_file = fopen(rockspath, "r");
//     assert(rocks_file != NULL);
//     strarray *info = get_split_line_from_file(rocks_file);
//     size_t num_rocks = atoi(info->data[0]);
//     level->rocks = list_init(num_rocks, NULL);
//     free_strarray(info);
//     double asteroid_center_x = 0;
//     double asteroid_center_y = 0;
//     double asteroid_radius = 0;
//     double num_sides = 0;
//     level->scene = scene_init_fixed_size(num_rocks + 1, 1, 1, 1);
//     for (size_t i = 0; i < num_rocks; i++) {
//         info = get_split_line_from_file(rocks_file);
//         asteroid_center_x = atoi(info->data[0]);
//         asteroid_center_y = atoi(info->data[1]);
//         asteroid_radius = atoi(info->data[2]);
//         num_sides = atoi(info->data[3]);
//         vector_t asteroid_center = {asteroid_center_x, asteroid_center_y};
//         list_t *asteroid = asteroid_init(asteroid_center, asteroid_radius, num_sides);
//         body_t *new_body = body_init(asteroid, 20, (rgb_color_t){.8, .8, .8});
//         scene_add_body(level->scene, new_body);
//         list_add(level->rocks, new_body);
//         free_strarray(info);
//     }
//     return level;
// }

FILE *helper_get_data(char*path, char* file_name) {
    char* abs_path = malloc(sizeof(char) * (strlen(path) + 11));
    strcat(abs_path, path);
    strcat(abs_path, file_name);
    printf("%s\n", abs_path);
    FILE *file = fopen(abs_path, "r");
    assert(file != NULL);
    return file;
}

void add_walls(level_t *level, char *path) {
    FILE *wall_file = helper_get_data(path, "/walls.dat");
    printf("burn\n");
    strarray *info = get_split_line_from_file(wall_file);
    size_t num_walls = atoi(info -> data[0]);
    level -> walls = list_init(num_walls, NULL);
    free_strarray(info);
    for (size_t i = 0; i < num_walls; i++) {
        info = get_split_line_from_file(wall_file);
        body_t *new_body = get_bodies_from_array(info);
        scene_add_body(level->scene, new_body);
        list_add(level->walls, (void *) new_body);
        free_strarray(info);
    }
}

void add_enemies(level_t *level, char *path) {
    printf("%s\n", path);
    FILE *enemy_file = helper_get_data(path, "/enemy.dat");
    printf("burn\n");
    strarray *info = get_split_line_from_file(enemy_file);
    size_t num_ships = atoi(info->data[0]) + 1;
    size_t num_dynamic = num_ships * (1 + MAX_PROJ_PER_SHIP);
    level->dynamic_objs = list_init(num_dynamic, NULL);
    free_strarray(info);
    for (size_t i = 0; i < num_ships - 1; i++) {
        info = get_split_line_from_file(enemy_file);
        body_t *new_body = get_bodies_from_array(info);
        scene_add_body(level->scene, new_body);
        list_add(level->dynamic_objs, (void *) new_body);
        free_strarray(info);
    }
}

void add_rocks(level_t *level, char *path) {
    FILE *rocks_file = helper_get_data(path, "/rocks.dat");
    printf("burn\n");
    strarray *info = get_split_line_from_file(rocks_file);
    size_t num_rocks = atoi(info->data[0]);
    level->rocks = list_init(num_rocks, NULL);
    free_strarray(info);
     double asteroid_center_x = 0;
    double asteroid_center_y = 0;
    double asteroid_radius = 0;
    size_t num_sides = 0;
    for (size_t i = 0; i < num_rocks; i++) {
        info = get_split_line_from_file(rocks_file);
        asteroid_center_x = atoi(info->data[0]);
        asteroid_center_y = atoi(info->data[1]);
        asteroid_radius = atoi(info->data[2]);
        num_sides = atoi(info->data[3]);
        vector_t asteroid_center = {asteroid_center_x, asteroid_center_y};
        list_t *asteroid = asteroid_outline_init(asteroid_center, asteroid_radius, num_sides);
        body_t *asteroid_body = body_init(asteroid, 20, (rgb_color_t){.5, .5, .5});
        vector_t dimple_center = body_get_centroid(asteroid_body);
        double dimples_radius = asteroid_radius / 2;
        double dimple_radius = asteroid_radius / 7;
        double dimple_x = dimple_center.x;
        double dimple_y = dimple_center.y;
        size_t num_dimples = (num_sides / 4) + 2;
        double dimple_angle = (2 * M_PI) / num_dimples;
        double angle = 0;
        for (size_t i = 0; i < num_dimples; i++){
            dimple_x = dimple_center.x + (dimples_radius * cos(angle));
            dimple_y = dimple_center.y + (dimples_radius * sin(angle));
            // dimple_center = (vector_t) {dimple_x, dimple_y};
            list_t *dimple = dimple_init((vector_t) {dimple_x, dimple_y}, dimple_radius);
            body_add_shape(asteroid_body, dimple, (rgb_color_t){.2, .2, .2});
            angle = angle + dimple_angle; 
        }
        scene_add_body(level->scene, asteroid_body);
        list_add(level->rocks, asteroid_body);
        free_strarray(info);
    }
}

level_t *level_init_from_folder(char *path) {
    level_t *level = malloc(sizeof(level));
    level -> scene = scene_init();
    add_walls(level, path);
    // add_enemies(level, path);
    add_rocks(level, path);
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
