#include "body.h"
#include "collision.h"
#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "background.h"
#include "weapon.h"
#include <math.h>
const size_t shape_steps = 200;
const double XMAX = 2000;
const double YMAX = 700;
const double UFO_MASS = 2;
const rgb_color_t UFO_COLOR = {1, 0, 0};
const size_t UFO_COLUMNS = 4;
const size_t UFO_ROWS = 1;
const double SHIP_MASS = 2;
const rgb_color_t SHIP_COLOR = {0, 0, 1};
const double UFO_VELO = 300;
const double PROJECTILE_MASS = 3;
const rgb_color_t PROJECTILE_COLOR = {0, 1, 0};
const double PROJECTILE_VELOCITY = 500;
const double SHIP_VELOCITY = 300;
const size_t NUM_ENEMIES = 24;
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
const size_t NUM_BACK_STARS = 50;
const size_t POSITION_APPROXIMATION_ORDER = 50;
const double ANG_VAR = 0.1;


typedef struct state {
  scene_t *scene;
  size_t loops;
} state_t;

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

void add_enemy_projectile(vector_t base, state_t *state) {
  char *info = malloc(7 * sizeof(char));
  strcpy(info, "ebullet");
  body_t *bod = body_init_with_info(projectile_init(base), PROJECTILE_MASS,
                                    PROJECTILE_COLOR, (void *)info, free);
  body_set_velocity(bod, (vector_t){0, -PROJECTILE_VELOCITY});
  scene_add_body(state->scene, bod);
}

void add_ship_projectile(vector_t base, state_t *state) {
  char *info = malloc(7 * sizeof(char));
  strcpy(info, "pbullet");
  body_t *bod = body_init_with_info(projectile_init(base), PROJECTILE_MASS,
                                    PROJECTILE_COLOR, (void *)info, free);
  body_set_velocity(bod, (vector_t){0, PROJECTILE_VELOCITY});
  scene_add_body(state->scene, bod);
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    if (!strcmp((char *)body_get_info(scene_get_body(state->scene, i)),
                "alien")) {
      create_destructive_collision(state->scene, bod,
                                   scene_get_body(state->scene, i));
    }
  }
}

list_t *UFO_init(vector_t center) {
  size_t steps = shape_steps;
  double dx = UFO_WIDTH / (float)steps;
  double x = UFO_WIDTH / 2;
  list_t *UFO = list_init(steps * 2, free);
  for (size_t i = 0; i < steps; i++) {
    vector_t *new_vec = malloc(sizeof(vector_t));
    if ((x <= UFO_WIDTH / 2 && x >= UFO_BUBBLE_UPPER_BOUND) ||
        (x <= (-1 * UFO_BUBBLE_UPPER_BOUND))) {
      *new_vec =
          (vector_t){x + center.x,
                     center.y + ((-1 * pow(x, 6)) / UFO_BUBBLE_WIDTH_CONSTANT)};
    } else {
      *new_vec =
          (vector_t){x + center.x,
                     center.y + (-1 * pow(x, 4) / UFO_SAUCER_WIDTH_CONSTANT) +
                         UFO_UPPER_OFFSET};
    }
    list_add(UFO, new_vec);
    x = x - dx;
  }
  x = -UFO_WIDTH / 2;
  for (size_t j = 0; j < steps; j++) {
    vector_t *new_vec = malloc(sizeof(vector_t));
    if ((x >= -UFO_WIDTH / 2 && x <= -UFO_LOWER_SAUCER_UPPER_BOUND) ||
        (x >= UFO_LOWER_SAUCER_UPPER_BOUND && x <= UFO_WIDTH / 2)) {
      *new_vec = (vector_t){x + center.x, center.y - UFO_LOWER_SAUCER_HEIGHT};
    } else if ((x > -UFO_LOWER_SAUCER_UPPER_BOUND && x < -UFO_WEAPON_BOUND) ||
               (x > UFO_WEAPON_BOUND && x < UFO_LOWER_SAUCER_UPPER_BOUND)) {
      *new_vec =
          (vector_t){x + center.x,
                     center.y + (pow(x, 16) / UFO_LOWER_SAUCER_WIDTH_CONSTANT) -
                         UFO_LOWER_OFFSET};
    } else {
      *new_vec = (vector_t){x + center.x,
                            center.y + (UFO_WEAPON_WIDTH_CONSTANT * fabs(x)) -
                                (UFO_WEAPON_OFFSET)};
    }
    list_add(UFO, new_vec);
    x = x + dx;
  }
  return UFO;
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

void create_enemy_row(state_t *state, double height) {
  for (size_t i = 0; i < UFO_COLUMNS; i++) {
    char *info = malloc(6 * sizeof(char));
    strcpy(info, "alien");
    body_t *new_bod = body_init_with_info(
        UFO_init(
            (vector_t){XMAX - (i * UFO_SPACING) - (UFO_WIDTH / 2), height}),
        UFO_MASS, UFO_COLOR, (void *)info, free);
    body_set_velocity(new_bod, (vector_t){-UFO_VELO, 0});
    scene_add_body(state->scene, new_bod);
  }
}

void check_enemy_x(state_t *state) {
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    vector_t cent = body_get_centroid(body);
    if (!strcmp((char *)body_get_info(body), "alien")) {
      if (((XMAX - (cent).x <= UFO_WIDTH / 2) &&
           (body_get_velocity(body).x > 0)) ||
          (((cent).x <= UFO_WIDTH / 2) && (body_get_velocity(body).x < 0))) {
        body_set_centroid(body, vec_add(body_get_centroid(body),
                                        (vector_t){0, -UFO_DOWN_TRANSLATION}));
        body_set_velocity(body, vec_multiply(-1, body_get_velocity(body)));
      }
    }
  }
}

void key_handle(char key, key_event_type_t type, double held_time,
                state_t *state) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      body_set_velocity(scene_get_body(state->scene, 0),
                        (vector_t){-SHIP_VELOCITY, 0});
      break;
    case RIGHT_ARROW:
      body_set_velocity(scene_get_body(state->scene, 0),
                        (vector_t){SHIP_VELOCITY, 0});
      break;
    case SPACE:
      fire_user_weapon(state -> scene);
      break;
    case DOWN_ARROW:
      change_user_weapon(state -> scene);
      break;
    }
  }
  if (type == KEY_RELEASED) {
    body_set_velocity(scene_get_body(state->scene, 0), (vector_t){0, 0});
  }
}

double determine_angle(vector_t origin, vector_t click) {
  click.y = YMAX - click.y;
  vector_t diff = vec_subtract(click, origin);
  if((diff.x > 0 && diff.y < 0) || (diff.x < 0 && diff.y > 0)) {
    if(diff.y > 0) {
      return acos(diff.y / norm(diff));
    }
    else {
      return -acos(diff.y / norm(diff));
    }
  }
  else if(diff.x > 0 && diff.y > 0) {
    return acos(-(diff.y / norm(diff))) + M_PI;
  }
  else {
    return acos(diff.y / norm(diff));
  }
}

void mouse_handle(char button, key_event_type_t type, double mouse_x, double mouse_y, double held_time, state_t *state) {
  if (type == KEY_PRESSED) {
    switch(button) {
      vector_t origin;
      case 0:
        origin = body_get_centroid(scene_get_body(state->scene, 0));
        body_set_rotation(scene_get_body(state->scene, 0), determine_angle(origin, (vector_t) {mouse_x, mouse_y}));
        fire_user_weapon(state -> scene);
      break;
    }
  }
}

void check_enemy_height(state_t *state) {
  if (strcmp(body_get_info(scene_get_body(state->scene, 0)), "SHIP")) {
    exit(0);
  }
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (body_get_centroid(body).y <= 40 &&
        !strcmp((char *)body_get_info(body), "alien")) {
      exit(0);
    }
    if ((body_get_centroid(body).y <= 0 || body_get_centroid(body).y >= YMAX) &&
        (!strcmp((char *)body_get_info(body), "pbullet")
        || !strcmp((char *)body_get_info(body), "ebullet"))) {
      scene_remove_body(state->scene, i);
    }
  }
}

void create_enemies(state_t *state) {
  for (size_t i = 0; i < UFO_ROWS; i++) {
    create_enemy_row(state, (YMAX - ((i * UFO_WIDTH_SPACING))) - UFO_WIDTH / 2);
  }
}

void create_ship(state_t *state) {
  char *info = malloc(5 * sizeof(char));
  strcpy(info, "SHIP");
  body_t *body = body_init_with_info(ship_init((vector_t){XMAX / 2, UFO_HEIGHT}), SHIP_MASS, SHIP_COLOR, (void *)info, free);
  list_t *other_shape = projectile_init(body_get_centroid(body));
  body_add_shape(body, other_shape, (rgb_color_t){1, 1, 1});
  scene_add_body(state->scene, body);
}

void shoot_as_ai(state_t *state, body_t *enemy) {
  // Goes to arbitrary order to approximate this, but 1-2 is probably enough
  body_t *player = scene_get_body(state->scene, 0);
  vector_t gap = vec_subtract(body_get_centroid(enemy),
                        body_get_centroid(player));
  double dt = sqrt(vec_dot(gap, gap)) / PROJECTILE_VELOCITY;
  for (size_t i = 1; i < POSITION_APPROXIMATION_ORDER; i++) {
    vector_t new_player_position = vec_add(body_get_centroid(player), 
              vec_multiply(dt * 0.5, body_get_velocity(player)));
    gap = vec_subtract(body_get_centroid(enemy),
                        new_player_position);
    dt = sqrt(vec_dot(gap, gap)) / PROJECTILE_VELOCITY;                    
  }
  double angle = atan2(-gap.y, -gap.x);
  angle += (rand() / RAND_MAX) * ANG_VAR;

  vector_t cent = body_get_centroid(enemy);
  add_enemy_projectile(vec_add(cent, vec_multiply(PROJECTILE_OFFSET,
    (vector_t){cos(angle), sin(angle)})), state);
  body_set_rotation(enemy, angle + M_PI / 2);
  body_t *projectile = scene_get_body(state->scene, scene_bodies(state->scene) - 1);
  body_set_velocity(projectile, vec_multiply(PROJECTILE_VELOCITY,
                    (vector_t){cos(angle), sin(angle)}));
  body_set_rotation(projectile, angle - M_PI / 2);
  create_destructive_collision(
      state->scene, scene_get_body(state->scene, 0), projectile);

}

state_t *emscripten_init() {
  seed_random();
  sdl_init(VEC_ZERO, (vector_t){XMAX, YMAX});
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->scene = scene_init();
  generate_back_stars(state -> scene, NUM_BACK_STARS, XMAX, YMAX);
  state->loops = 0;
  create_ship(state);
  weapon_t *weapon = weapon_init((void *)projectile_init, PROJECTILE_VELOCITY, PROJECTILE_COLOR, PROJECTILE_MASS, scene_get_body(state -> scene, 0));
  weapon_t *weapon2 = weapon_init((void *)ship_init, PROJECTILE_VELOCITY, PROJECTILE_COLOR, PROJECTILE_MASS, scene_get_body(state -> scene, 0));
  add_user_weapon(state -> scene, weapon, create_destructive_collision);
  add_user_weapon(state -> scene, weapon2, create_destructive_collision);
  create_enemies(state);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_on_click(mouse_handle);
  sdl_on_key(key_handle);
  state->loops += 1;
  if (gen_rand(0, ENEMY_FIRE_RATE_RAND_MAX) < ENEMY_FIRE_RATE_CONTROL &&
      scene_bodies(state->scene) > 1) {
    body_t *body = scene_get_body(state->scene,
                                  gen_rand(1, scene_bodies(state->scene) - 1));
    while (strcmp((char *)body_get_info(body), "alien")) {
      body = scene_get_body(state->scene,
                            gen_rand(1, scene_bodies(state->scene) - 1));
    }
    shoot_as_ai(state, body);
    
  }
  double dt = time_since_last_tick();
  check_enemy_x(state);
  sdl_render_scene(state->scene);
  check_enemy_height(state);
  scene_tick(state->scene, dt);
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}