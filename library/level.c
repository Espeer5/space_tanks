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
const size_t CAN_POINTS = 50;
const double CAN_RAD = 20;
const double CAN_MASS = 50;
const double CAN_VELO = 420;
const vector_t START = {250, 500};
const size_t shape_steps = 200;
const double SHIP_MASS = 2;
const rgb_color_t SHIP_COLOR = {0, 0, 1};
const double PROJECTILE_MASS = .1;
const double ASTEROID_MASS = 20;
const rgb_color_t PROJECTILE_COLOR = {0, 1, 0};
const double PROJECTILE_VELOCITY = 1000;
const double PROJECTILE_OFFSET =
    9; // How far from a body does its projectile spawn
const double PROJECTILE_WIDTH = 6;
const double SHIP_WIDTH = 80;
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
const double ENEMY_FIRE_RATE_RAND_MAX = 1000;
const double ENEMY_FIRE_RATE_CONTROL =
    990; // Lower number to fire less frequently, higher to fire more frequenctly
const size_t POSITION_APPROXIMATION_ORDER = 50;
const double ANG_VAR = 0.1;
const rgb_color_t HITBOX_COL = {0, 0, 0};
const double ENEMY_SHOT_ELASTICITY = .7;
const double SHUR_SIDES = 6;
const double SHUR_RAD = 10;
const double SHIP_HITBOX_SCALE = 1.5;
const rgb_color_t ENEMY_COLOR = {1, 0, 0};
const double USER_WEAPON_ELASTICITY = .8;

typedef struct level {
    scene_t *scene;
    list_t *dynamic_objs;
    list_t *rocks;
    body_t *user;
} level_t;


scene_t *level_scene(level_t *level) {
    return level->scene;
}

size_t level_rocks(level_t *level) {
    return list_size(level -> rocks);
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
    body_t *enemy = body_init_with_info(box_init((vector_t) {(double) atoi(arr -> data[0]), (double) atoi(arr -> data[1])}, SHIP_WIDTH/1.5, SHIP_WIDTH/1.5), SHIP_MASS, HITBOX_COL, info, free);
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
    char* abs_path = malloc(sizeof(char) * (strlen(path) + strlen(file_name) + 1));
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
  body_t *player = scene_get_body(level_scene(level), 0);
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
  angle += ((double) rand() / RAND_MAX) * ANG_VAR;
  body_set_rotation(scene_get_body(level_scene(level), enemy_num), angle - M_PI / 2);
  body_t *en_proj = fire_enemy_weapon(scene, enemy_num);
  create_destructive_collision(level_scene(level), scene_get_body(level_scene(level), 0), en_proj);
  for(size_t t = 0; t < scene_bodies(level_scene(level)); t++) {
        body_t *rock = scene_get_body(level_scene(level), t);
        create_physics_collision(level_scene(level), ENEMY_SHOT_ELASTICITY, en_proj, rock);
  }
}

list_t *shuriken_init(vector_t center) {
  double d_angle = (M_PI / SHUR_SIDES);
  double total_angle = 0;
  size_t counter = 2;
  double scaler = sin(M_PI / (2 * SHUR_SIDES)) /
                  sin(((SHUR_SIDES - 3) * M_PI) / SHUR_SIDES);
  list_t *star = list_init(2 * SHUR_SIDES, free);
  for (size_t i = 0; i < 2 * SHUR_SIDES; i++) {
    total_angle = total_angle + d_angle;
    vector_t *new_vec_pointer = malloc(sizeof(vector_t));
    assert(new_vec_pointer != NULL);
    if (counter % 2) {
      *new_vec_pointer =
          (vector_t){center.x - (SHUR_RAD * scaler * cos(total_angle)),
                     center.y - (SHUR_RAD * scaler * sin(total_angle))};

    } else {
      *new_vec_pointer =
          (vector_t){center.x - (SHUR_RAD * cos(total_angle)),
                     center.y - (SHUR_RAD * sin(total_angle))};
    }
    list_add(star, new_vec_pointer);
    counter++;
  }
  return star;
}

list_t *canon_init(vector_t center) {
  list_t *c = list_init(CAN_POINTS, free);
  for (size_t i = 0; i < CAN_POINTS; i++) {
    double angle = i * ((2 * M_PI ) / CAN_POINTS);
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t){center.x + (CAN_RAD * cos(angle)),
                    center.y + (CAN_RAD * sin(angle))};
    list_add(c, v);
  }
  return c;
}

void int_ship(level_t *level) {
    list_t *user_ship = ship_init(START);
    char *info1 = malloc(5 * sizeof(char));
    strcpy(info1, "ship");
    body_t *user_bod = body_init_with_info(box_init(START, SHIP_WIDTH/SHIP_HITBOX_SCALE, SHIP_WIDTH/SHIP_HITBOX_SCALE), SHIP_MASS, HITBOX_COL, info1, free);
    body_add_shape(user_bod, user_ship, SHIP_COLOR);
    level -> user = user_bod;
    scene_add_front(level -> scene, user_bod);
    wipe_weapons(level_scene(level));
    rearm(level_scene(level));
    weapon_t *weapon1 = weapon_init((void *)projectile_init, PROJECTILE_VELOCITY, PROJECTILE_COLOR, PROJECTILE_MASS, level -> user);
    add_user_weapon(level -> scene, weapon1, create_destructive_collision);
    weapon_t *weapon2 = weapon_init((void *)shuriken_init, PROJECTILE_VELOCITY, PROJECTILE_COLOR, PROJECTILE_MASS, level -> user);
    weapon_t *weapon3 = weapon_init((void *)canon_init, CAN_VELO, PROJECTILE_COLOR, CAN_MASS, level -> user);
    add_user_weapon(level -> scene, weapon2, create_destructive_collision);
    add_user_weapon(level -> scene, weapon3, create_destructive_collision);
    for(size_t j = 0; j < scene_bodies(level -> scene); j++) {
        create_physics_collision(level -> scene, USER_WEAPON_ELASTICITY, user_bod, scene_get_body(level -> scene, j));
    }
}

level_t *level_init_from_folder(char *path, double XMAX, double YMAX) {
    level_t *level = malloc(sizeof(level_t));
    list_t *user_ship = ship_init(START);
    char *info1 = malloc(5 * sizeof(char));
    strcpy(info1, "ship");
    body_t *user_bod = body_init_with_info(box_init(START, SHIP_WIDTH/SHIP_HITBOX_SCALE, SHIP_WIDTH/SHIP_HITBOX_SCALE), SHIP_MASS, HITBOX_COL, info1, free);
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
    weapon_t *weapon2 = weapon_init((void *)shuriken_init, PROJECTILE_VELOCITY, PROJECTILE_COLOR, PROJECTILE_MASS, level -> user);
    weapon_t *weapon3 = weapon_init((void *)canon_init, CAN_VELO, PROJECTILE_COLOR, CAN_MASS, level -> user);
    add_user_weapon(level -> scene, weapon1, create_destructive_collision);
    add_user_weapon(level -> scene, weapon2, create_destructive_collision);
    add_user_weapon(level -> scene, weapon3, create_destructive_collision);
     for (size_t i = 0; i < num_ships - 1; i++) {
         info = get_split_line_from_file(enemy_file);
         body_t *new_body = get_bodies_from_array(info);
         scene_add_body(level->scene, new_body);
         weapon_t *w = weapon_init((void *)projectile_init, PROJECTILE_VELOCITY, ENEMY_COLOR, PROJECTILE_MASS, new_body);
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
            list_t *dimple = dimple_init((vector_t) {dimple_x, dimple_y}, dimple_radius);
            body_add_shape(asteroid_body, dimple, (rgb_color_t){.2, .2, .2});
            angle = angle + dimple_angle; 
        }
        scene_add_body(level->scene, asteroid_body);
        list_add(level->rocks, asteroid_body);
    }
    free_strarray(info);
    for(size_t t = 0; t < list_size(level -> rocks); t++) {
        body_t *rock = list_get(level -> rocks, t);
        for(size_t x = 0; x < scene_bodies(level -> scene); x++) {
            if(x != t) {
                create_physics_collision(level -> scene, .8, rock, scene_get_body(level -> scene, x));
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


void level_tick(level_t *level, double dt) {
    scene_tick_forces(level->scene);
    size_t naliens = 0;
    for (size_t i = 0; i < scene_bodies(level->scene); i++) {
        if (!strcmp((char *)body_get_info(scene_get_body(level->scene, i)),
                    "alien")) {
            if (body_is_removed(scene_get_body(level->scene, i))) {
                remove_enemy_weapon(level_scene(level), naliens);
            }
            naliens++;
        }
    }
    scene_tick_after_forces(level->scene, dt);
}
