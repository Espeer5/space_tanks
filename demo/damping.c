#include "body.h"
#include "forces.h"
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
const size_t CIRC_PPOINTS = 100;
const size_t NUM_CIRC = 40;
const double CIRC_RADIUS = 12.5;
const double CIRC_MASS = 1;
const double INFINITE_MASS = 1E10;
const double k = 5;
const int CMAX = 255;
const double GAMMA = 0.01;
const vector_t IMPULSE = {0, 300};

typedef struct state {
  size_t loops;
  scene_t *bodies;
} state_t;

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

void populate_window(scene_t *scene) {
  body_t *left_circle =
      body_init(make_circ((vector_t){-CIRC_RADIUS, YMAX / 2}, CIRC_RADIUS),
                INFINITE_MASS, rand_color());
  body_t *right_circle = body_init(
      make_circ((vector_t){XMAX + CIRC_RADIUS, YMAX / 2}, CIRC_RADIUS),
      INFINITE_MASS, rand_color());
  scene_add_body(scene, left_circle);
  for (size_t i = 0; i < NUM_CIRC; i++) {
    body_t *circle = body_init(
        make_circ(
            (vector_t){(2 * CIRC_RADIUS * i + CIRC_RADIUS),
                       (YMAX / 2 * (1 + sin(M_PI * (i + 1) / (NUM_CIRC + 2))))},
            CIRC_RADIUS),
        CIRC_MASS, rand_color());
    scene_add_body(scene, circle);
    create_drag(scene, GAMMA, circle);
  }
  scene_add_body(scene, right_circle);
  for (size_t i = 0; i < NUM_CIRC + 2; i++) {
    if (i > 0) {
      create_spring(scene, k, scene_get_body(scene, i - 1),
                    scene_get_body(scene, i));
    }
  }
}

state_t *emscripten_init() {
  seed_random();
  sdl_init(VEC_ZERO, (vector_t){XMAX, YMAX});
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->bodies = scene_init();
  state->loops = 0;
  populate_window(state->bodies);
  return state;
}

void emscripten_main(state_t *state) {
  state->loops += 1;
  double dt = time_since_last_tick();
  sdl_render_scene(state->bodies);
  scene_tick(state->bodies, dt);
}

void emscripten_free(state_t *state) {
  scene_free(state->bodies);
  free(state);
}