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
#include "forces.h"
#include "sdl_wrapper.h"

const int MAX_LINE_LENGTH = 100;
const int MAX_PROJ_PER_SHIP = 4;
const vector_t ship_pos = {50, 50};
const double STAR_RADIUS = 85;
const size_t STAR_POINTS = 5;
const double RED = 0;
const double GREEN = .1;
const double BLUE = 1;
const rgb_color_t color = {0, .1, .1};
const vector_t START = {250, 500};
const vector_t initial_velo = {300, -200};
const double omega = .05;
const size_t shape_steps = 200;
const double UFO_MASS = 2;
const rgb_color_t UFO_COLOR = {1, 0, 0};
const size_t UFO_COLUMNS = 4;
const size_t UFO_ROWS = 1;
const double SHIP_MASS = 2;
const rgb_color_t SHIP_COLOR = {0, 0, 1};
const double UFO_VELO = 300;
const double PROJECTILE_MASS = 5;
const double ASTEROID_MASS = 1;
const rgb_color_t PROJECTILE_COLOR = {0, 1, 0};
const double PROJECTILE_VELOCITY = 1000;
const size_t NUM_ENEMIES = 5;
const double PROJECTILE_OFFSET =
    9; // How far from a body does its projectile spawn
const double PROJECTILE_WIDTH = 6;
const double UFO_WIDTH = 80;
const double SHIP_WIDTH = 80;
const double UFO_BUBBLE_UPPER_BOUND = 8.801;
const double UFO_BUBBLE_WIDTH_CONSTANT = 3e8;
const double UFO_SAUCER_WIDTH_CONSTANT = 300;
const double UFO_LOWER_SAUCER_UPPER_BOUND = 14.618;
const double UFO_LOWER_SAUCER_HEIGHT = 13.653;
const double UFO_LOWER_SAUCER_WIDTH_CONSTANT = 10e18;
const double UFO_LOWER_OFFSET = 18;
const double UFO_UPPER_OFFSET = 20;
const double UFO_WEAPON_WIDTH_CONSTANT = 3;
const double UFO_WEAPON_OFFSET = 27;
const double UFO_DOWN_TRANSLATION = 200;
const double UFO_SPACING = 100;
const double UFO_WEAPON_BOUND = 3;
const double SHIP_COCKPIT_BOUND = 6.961;
const double SHIP_COCKPIT_WIDTH_CONSTANT = 100;
const double SHIP_WEAPON_BOUND = 21.579;
const double SHIP_WEAPON_OUTSIDE_EDGE = 18.571;
const double SHIP_WEAPON_WIDTH_CONSTANT = 10;
const double SHIP_WEAPON_OFFSET = 20;
const double SHIP_WEAPON_HEIGHT = 25;
const double SHIP_WING_WIDTH_CONSTANT = .5;
const double SHIP_WING_SLANT_BOUND = 5.6;
const double SHIP_WING_SLANT_OUTER_BOUND = 28;
const double SHIP_AIR_FOIL_WIDTH_CONSTANT = .25;
const double SHIP_AIRFOIL_OFFSET = 7;
const double UFO_WIDTH_SPACING = 70;
const double UFO_HEIGHT = 60;
const double ENEMY_FIRE_RATE_RAND_MAX = 1000;
const double ENEMY_FIRE_RATE_CONTROL =
    400; // Lower number to fire less frequently, higher to fire more frequenctly
const size_t POSITION_APPROXIMATION_ORDER = 50;
const double ANG_VAR = 0.1;


/**
 * A generalization of scene to .
 * The level automatically resizes to store
 * arbitrarily many dynamic and force creators, but has a static
 * number of background objects, and a finite list of 
 * walls.
 */
typedef struct level {
    scene_t *scene;
    list_t *dynamic_objs;
    list_t *rocks;
    body_t *user;
    key_handler_t *key_handle;
    mouse_handler_t *mouse_handle;
} level_t;


scene_t *level_scene(level_t *level) {
    return level->scene;
}

void free_traj(trajectory_t *traj) {
    list_free(traj->positions);
    free(traj);
}

size_t level_rocks(level_t *level) {
    return list_size(level -> rocks);
}

list_t *level_get_rocks(level_t *level) {
    return level -> rocks;
}

void set_key_handler(level_t *level, key_handler_t *handler) {
    level -> key_handle = handler;
}

key_handler_t *get_key_handler(level_t *level) {
    return level -> key_handle;
}

void set_mouse_handle(level_t *level, mouse_handler_t *handler) {
    level -> mouse_handle = handler;
}

mouse_handler_t *get_mouse_handle(level_t *level) {
    return level -> mouse_handle;
}


list_t *ship_init(vector_t origin) {
  size_t steps = shape_steps;
  double dx = SHIP_WIDTH / (float)steps;
  double x = SHIP_WIDTH / 2;
  list_t *ship = list_init(steps * 2, free);
  for (size_t i = 0; i < steps; i++) {
    vector_t *new_vec = malloc(sizeof(vector_t));
    if (-SHIP_COCKPIT_BOUND < x && x < SHIP_COCKPIT_BOUND) {
      *new_vec = (vector_t){
          origin.x + x,
          origin.y + ((-1 * pow(x, 4) / SHIP_COCKPIT_WIDTH_CONSTANT) +
                      SHIP_WIDTH / 2)};
    } else if (-SHIP_WEAPON_BOUND < x && x < -SHIP_WEAPON_OUTSIDE_EDGE) {
      *new_vec = (vector_t){x + origin.x,
                            origin.y + (-SHIP_WEAPON_WIDTH_CONSTANT *
                                            fabs(x + SHIP_WEAPON_OFFSET) +
                                        SHIP_WEAPON_HEIGHT)};
    } else if (SHIP_WEAPON_OUTSIDE_EDGE < x && x < SHIP_WEAPON_BOUND) {
      *new_vec = (vector_t){origin.x + x,
                            origin.y + (-SHIP_WEAPON_WIDTH_CONSTANT *
                                            fabs(x - SHIP_WEAPON_OFFSET) +
                                        SHIP_WEAPON_HEIGHT)};
    } else {
      *new_vec = (vector_t){origin.x + x,
                            origin.y + (-SHIP_WING_WIDTH_CONSTANT * fabs(x) +
                                        SHIP_WEAPON_OFFSET)};
    }
    list_add(ship, new_vec);
    x = x - dx;
  }
  x = -SHIP_WIDTH / 2;
  for (size_t j = 0; j < steps; j++) {
    vector_t *new_vec = malloc(sizeof(vector_t));
    if (-SHIP_WING_SLANT_BOUND < x && x < SHIP_WING_SLANT_BOUND) {
      *new_vec = (vector_t){origin.x + x, origin.y + fabs(x)};
    } else if ((-SHIP_WING_SLANT_OUTER_BOUND < x &&
                -SHIP_WING_SLANT_BOUND > x) ||
               (SHIP_WING_SLANT_BOUND < x && x < SHIP_WING_SLANT_OUTER_BOUND)) {
      *new_vec = (vector_t){
          origin.x + x, origin.y + (-SHIP_AIR_FOIL_WIDTH_CONSTANT * fabs(x) +
                                    SHIP_AIRFOIL_OFFSET)};
    } else {
      *new_vec = (vector_t){origin.x + x, origin.y};
    }
    list_add(ship, new_vec);
    x = x + dx;
  }
  return ship;
}

list_t *box_init(vector_t center, double width, double height) {
    list_t *box = list_init(4, free);
    vector_t point1 = {center.x - width/2, center.y + height/2};
    vector_t point2 = {center.x + width/2, center.y + height/2};
    vector_t point3 = {center.x + width/2, center.y - height/2};
    vector_t point4 = {center.x - width/2, center.y - height/2};
    vector_t *vec1 = malloc(sizeof(vector_t));
    *vec1 = point1;
    vector_t *vec2 = malloc(sizeof(vector_t));
    *vec2 = point2;
    vector_t *vec3 = malloc(sizeof(vector_t));
    *vec3 = point3;
    vector_t *vec4 = malloc(sizeof(vector_t));
    *vec4 = point4;
    list_add(box, vec1);
    list_add(box, vec2);
    list_add(box, vec3);
    list_add(box, vec4);
    return box;
}



body_t *get_bodies_from_array(strarray *arr) {
    char *info = malloc(6 * sizeof(char));
    strcpy(info, "alien");
    list_t *bod_graphics = ship_init((vector_t) {(double) atoi(arr -> data[0]), (double) atoi(arr -> data[1])});
    body_t *enemy = body_init_with_info(box_init((vector_t) {(double) atoi(arr -> data[0]), (double) atoi(arr -> data[1])}, SHIP_WIDTH/1.5, SHIP_WIDTH/1.5), SHIP_MASS, (rgb_color_t) {0, 0, 0}, info, free);
    body_add_shape(enemy, bod_graphics, (rgb_color_t) {1, 0, 0});
    return enemy;
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


FILE *helper_get_data(char*path, char* file_name) {
    char* abs_path = malloc(sizeof(char) * (strlen(path) + 11));
    strcpy(abs_path, path);
    strcat(abs_path, file_name);
    FILE *file = fopen(abs_path, "r");
    assert(file != NULL);
    return file;
}

list_t *projectile_init(vector_t base) {
  list_t *projectile = list_init(4, free);
  vector_t *vert1 = malloc(sizeof(vector_t));
  *vert1 =
      (vector_t){base.x - PROJECTILE_WIDTH / 2, base.y + PROJECTILE_OFFSET};
  list_add(projectile, vert1);
  vector_t *vert2 = malloc(sizeof(vector_t));
  *vert2 =
      (vector_t){base.x + PROJECTILE_WIDTH / 2, base.y + PROJECTILE_OFFSET};
  list_add(projectile, vert2);
  vector_t *vert3 = malloc(sizeof(vector_t));
  *vert3 =
      (vector_t){base.x + PROJECTILE_WIDTH / 2, base.y - PROJECTILE_OFFSET};
  list_add(projectile, vert3);
  vector_t *vert4 = malloc(sizeof(vector_t));
  *vert4 =
      (vector_t){base.x - PROJECTILE_WIDTH / 2, base.y - PROJECTILE_OFFSET};
  list_add(projectile, vert4);
  return projectile;
}

void shoot_as_ai(level_t *level, size_t enemy_num) {
  // Goes to arbitrary order to approximate this, but 1-2 is probably enough
  scene_t *scene = level_scene(level);
  body_t *player = level -> user;
  vector_t gap = vec_subtract(body_get_centroid(scene_get_body(level_scene(level), enemy_num)),
                        body_get_centroid(player));
  double dt = sqrt(vec_dot(gap, gap)) / PROJECTILE_VELOCITY;
  for (size_t i = 1; i < POSITION_APPROXIMATION_ORDER; i++) {
    vector_t new_player_position = vec_add(body_get_centroid(player), 
              vec_multiply(dt * 0.5, body_get_velocity(player)));
    gap = vec_subtract(body_get_centroid(scene_get_body(level_scene(level), enemy_num)),
                        new_player_position);
    dt = sqrt(vec_dot(gap, gap)) / PROJECTILE_VELOCITY;                    
  }
  double angle = atan2(-gap.y, -gap.x);
  angle += (rand() / RAND_MAX) * ANG_VAR;
  body_set_rotation(scene_get_body(level_scene(level), enemy_num), angle + M_PI / 2);
  fire_enemy_weapon(scene, enemy_num);
}

level_t *level_init_from_folder(char *path, double XMAX, double YMAX) {
    level_t *level = malloc(sizeof(level));
    list_t *user_ship = ship_init(START);
    body_t *user_bod = body_init(box_init(START, SHIP_WIDTH/1.5, SHIP_WIDTH/1.5), SHIP_MASS, (rgb_color_t) {0, 0, 0});
    body_add_shape(user_bod, user_ship, SHIP_COLOR);
    level -> user = user_bod;
    FILE *enemy_file = helper_get_data(path, "/enemy.dat");
    FILE *rocks_file = helper_get_data(path, "/rocks.dat");
    strarray *info = get_split_line_from_file(enemy_file);
    size_t num_ships = atoi(info->data[0]) + 1;
    size_t num_dynamic = num_ships * (1 + MAX_PROJ_PER_SHIP);
    level->dynamic_objs = list_init(num_dynamic, NULL);
    free_strarray(info);
    info = get_split_line_from_file(rocks_file);
    size_t num_rocks = atoi(info->data[0]);
    level->rocks = list_init(num_rocks, NULL);
    free_strarray(info);
    level->scene = scene_init_fixed_size(num_dynamic + num_rocks,
                                 num_dynamic * (num_dynamic), 1, 1);
    scene_add_body(level -> scene, level -> user);
    weapon_t *weapon1 = weapon_init((void *)projectile_init, PROJECTILE_VELOCITY, PROJECTILE_COLOR, PROJECTILE_MASS, level -> user);
    add_user_weapon(level -> scene, weapon1, create_destructive_collision);
     for (size_t i = 0; i < num_ships - 1; i++) {
         info = get_split_line_from_file(enemy_file);
         body_t *new_body = get_bodies_from_array(info);
         scene_add_body(level->scene, new_body);
         weapon_t *w = weapon_init((void *)projectile_init, PROJECTILE_VELOCITY, (rgb_color_t) {1, 0, 0}, PROJECTILE_MASS, new_body);
         add_enemy_weapon(level_scene(level), w, create_destructive_collision);
         list_add(level->dynamic_objs, (void *) new_body);
         free_strarray(info);
     }
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
        char *info = malloc(5 * sizeof(char));
        strcpy(info, "rock");
        body_t *asteroid_body = body_init_with_info(box_init(asteroid_center, asteroid_radius * cos(M_PI/num_sides), asteroid_radius * cos(M_PI/num_sides)), ASTEROID_MASS, (rgb_color_t){1, 1, 1}, info, free);
        body_add_shape(asteroid_body, asteroid, (rgb_color_t){.5, .5, .5});
        vector_t dimple_center = body_get_centroid(asteroid_body);
        double dimples_radius = asteroid_radius / 2;
        double dimple_radius = asteroid_radius / 15;
        double dimple_x = dimple_center.x;
        double dimple_y = dimple_center.y;
        size_t num_dimples = num_sides;
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
        create_physics_collision(level -> scene, 1, asteroid_body, level->user);
        list_add(level->rocks, asteroid_body);
    }
    free_strarray(info);
    for(size_t t = 0; t < list_size(level -> rocks); t++) {
        body_t *rock = list_get(level -> rocks, t);
        for(size_t l = 0; l < list_size(level -> dynamic_objs); l++) {
            create_destructive_collision(level -> scene, rock, list_get(level -> dynamic_objs, l));
        }
        for(size_t x = 0; x < list_size(level -> rocks); x++) {
            if(x != t) {
                create_physics_collision(level -> scene, .8, rock, list_get(level -> rocks, x));
            }
        }
    }
    fclose(enemy_file);
    fclose(rocks_file);
    return level;
}

void level_free(level_t *level) {
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
    scene_tick_after_forces(level->scene, dt);
}
