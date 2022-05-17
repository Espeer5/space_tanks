#include "body.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
const double XMAX = 1000;
const double YMAX = 500;
const size_t CIRC_PPOINTS = 65;
const size_t FOODS = 25;
const double FOOD_RAD = 13;
const double FOOD_MASS = 1;
const double PAC_MASS = 1;
const rgb_color_t YELLOW = {1, 1, 0};
const vector_t PAC_START = {500, 250};
const double PAC_RADIUS = 55;
const size_t FOOD_CYCLES = 200;
const double PAC_ACC = 400;
const double PAC_INT_VELO = 100;

typedef struct state {
  size_t loops;
  scene_t *bodies;
} state_t;

list_t *make_pac(vector_t center, double radius) {
  list_t *pac = list_init(CIRC_PPOINTS, free);
  vector_t *p1 = malloc(sizeof(vector_t));
  *p1 = (vector_t){center.x, center.y};
  list_add(pac, p1);
  double d_angle = ((2 * M_PI) - (M_PI / 4)) / CIRC_PPOINTS;
  double total_angle = M_PI / 8;
  while (total_angle <= 15 * M_PI / 8) {
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t){center.x + (radius * cos(total_angle)),
                    center.y + (radius * sin(total_angle))};
    list_add(pac, v);
    total_angle += d_angle;
  }
  return pac;
}

void pac_go(state_t *state, double direction, vector_t velo, vector_t acc,
            double hold_time) {
  body_t *pac = scene_get_body(state->bodies, 0);
  list_t *pac_shape = body_get_shape(scene_get_body(state->bodies, 0));
  body_set_rotation(pac, direction);
  body_set_velocity(pac, vec_add(velo, vec_add(body_get_velocity(pac),
                                               vec_multiply(hold_time, acc))));
  list_free(pac_shape);
}

void key_handle(char key, key_event_type_t type, double held_time,
                state_t *state) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      pac_go(state, M_PI, (vector_t){-PAC_INT_VELO, 0}, (vector_t){-PAC_ACC, 0},
             held_time);
      break;
    case RIGHT_ARROW:
      pac_go(state, 0, (vector_t){PAC_INT_VELO, 0}, (vector_t){PAC_ACC, 0},
             held_time);
      break;
    case UP_ARROW:
      pac_go(state, M_PI / 2, (vector_t){0, PAC_INT_VELO},
             (vector_t){0, PAC_ACC}, held_time);
      break;
    case DOWN_ARROW:
      pac_go(state, 3 * M_PI / 2, (vector_t){0, -PAC_INT_VELO},
             (vector_t){0, -PAC_ACC}, held_time);
      break;
    }
  }
  if (type == KEY_RELEASED) {
    body_set_velocity(scene_get_body(state->bodies, 0), (vector_t){0, 0});
  }
}

list_t *make_circ(vector_t center, double radius) {
  list_t *c = list_init(CIRC_PPOINTS, free);
  for (size_t i = 0; i < CIRC_PPOINTS; i++) {
    double angle = 2 * M_PI * i / CIRC_PPOINTS;
    vector_t *v = malloc(sizeof(*v));
    *v = (vector_t){center.x + (radius * cos(angle)),
                    center.y + (radius * sin(angle))};
    list_add(c, v);
  }
  return c;
}

double loop_position(double position, double min, double max) {
  // Note: this doesn't account for going way out of bounds by multiple times
  if (position < min) {
    return position + (max - min);
  }
  if (position > max) {
    return position - (max - min);
  }
  return position;
}

void advance_time_pac(state_t *state, double dt) {
  body_t *pac_body = scene_get_body(state->bodies, 0);
  body_tick(pac_body, dt);
  list_t *pac_shape = body_get_shape(scene_get_body(state->bodies, 0));
  vector_t pac_centroid = body_get_centroid(pac_body);
  vector_t pac_position = *((vector_t *)list_get(pac_shape, 0));
  list_free(pac_shape);
  vector_t diff = vec_subtract(pac_centroid, pac_position);
  pac_position.x = loop_position(pac_position.x, 0, XMAX);
  pac_position.y = loop_position(pac_position.y, 0, YMAX);
  body_set_centroid(pac_body, vec_add(diff, pac_position));
  for (size_t i = scene_bodies(state->bodies) - 1; i > 0; i--) {
    if (i > 0 && i < scene_bodies(state->bodies) - 1) {
      body_t *pellet = scene_get_body(state->bodies, i);
      vector_t distance = vec_subtract(body_get_centroid(pellet), pac_position);
      double distance_mag = sqrt(vec_dot(distance, distance));
      if (distance_mag < PAC_RADIUS) {
        scene_remove_body(state->bodies, i);
      }
    }
  }
}

void populate_window(scene_t *scene) {
  for (size_t i = 0; i < FOODS; i++) {
    body_t *food =
        body_init(star_init(5, (vector_t){gen_rand(0, XMAX), gen_rand(0, YMAX)},
                            FOOD_RAD),
                  FOOD_MASS, YELLOW);
    scene_add_body(scene, food);
  }
}

state_t *emscripten_init() {
  srand(time(0));
  sdl_init(VEC_ZERO, (vector_t){XMAX, YMAX});
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->bodies = scene_init();
  state->loops = 0;
  scene_add_body(state->bodies,
                 body_init(make_pac(PAC_START, PAC_RADIUS), PAC_MASS, YELLOW));
  body_set_rotation(scene_get_body(state->bodies, 0), 0);
  populate_window(state->bodies);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_on_key(key_handle);
  state->loops += 1;
  double dt = time_since_last_tick();
  sdl_render_scene(state->bodies);
  advance_time_pac(state, dt);
  if (!(state->loops % FOOD_CYCLES) && state->loops != 0) {
    populate_window(state->bodies);
  }
}

void emscripten_free(state_t *state) {
  scene_free(state->bodies);
  free(state);
}