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
const size_t NUM_STARS = 200;
const double STAR_RAD = 80;
const double G = 1000;
const int CMAX = 255;

typedef struct state {
  size_t loops;
  scene_t *bodies;
} state_t;

void populate_window(scene_t *scene) {
  body_t *black_hole = body_init(star_init(4, (vector_t){500, 250}, 1), 500,
                                 (rgb_color_t){0, 0, 0});
  scene_add_body(scene, black_hole);
  for (size_t i = 0; i < NUM_STARS; i++) {
    double radius = gen_rand(10, STAR_RAD);
    body_t *star =
        body_init(star_init(4, (vector_t){gen_rand(0, XMAX), gen_rand(0, YMAX)},
                            gen_rand(5, 20)),
                  (radius / STAR_RAD) * (radius / STAR_RAD), rand_color());
    scene_add_body(scene, star);
    for (size_t j = 0; j < i; j++) {
      create_newtonian_gravity(scene, G, scene_get_body(scene, j),
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