#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include "level.h"
#include "utils.h"

const double XMAX = 2000;
const double YMAX = 1000;
const size_t BACK_STARS = 100;

typedef struct state {
  level_t *level;
} state_t;



state_t *emscripten_init() {
  vector_t min = (vector_t){0, 0};
  vector_t max = (vector_t){XMAX, YMAX};
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  state -> level = level_init_from_folder("/levels/level1", XMAX, YMAX);
  scene_add_body(level_scene(state->level), body_init(make_square(), 1, (rgb_color_t) {1,0,0}));
  generate_back_stars(level_scene(state -> level), BACK_STARS, XMAX, YMAX);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_render_scene(level_scene(state->level));
}

void emscripten_free(state_t *state) {
  free(state);
}
