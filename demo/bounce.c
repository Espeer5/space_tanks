#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>

const double XMAX = 1000;
const double YMAX = 500;
const double STAR_RADIUS = 85;
const size_t STAR_POINTS = 5;
const double RED = 0;
const double GREEN = .1;
const double BLUE = 1;
const rgb_color_t color = {0, .1, .1};
const vector_t START = {250, 500};
const vector_t initial_velo = {300, -200};
const double omega = .05;

typedef struct state {
  list_t *star;
  vector_t velocity;
} state_t;

state_t *emscripten_init() {
  vector_t min = (vector_t){0, 0};
  vector_t max = (vector_t){XMAX, YMAX};
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->star = star_init(STAR_POINTS, START, STAR_RADIUS);
  state->velocity = initial_velo;
  return state;
}

bool check_x_bounds(state_t *state) {
  for (size_t i = 0; i < list_size(state->star); i++) {
    double x_comp = (*((vector_t *)list_get(state->star, i))).x;
    if ((0 > x_comp && (state->velocity).x < 0) ||
        ((state->velocity).x > 0 && x_comp > XMAX)) {
      return 1;
    }
  }
  return 0;
}

bool check_y_bounds(state_t *state) {
  for (size_t i = 0; i < list_size(state->star); i++) {
    double y_comp = (*((vector_t *)list_get(state->star, i))).y;
    if ((0 > y_comp && (state->velocity).y < 0) ||
        ((state->velocity).y > 0 && y_comp > YMAX)) {
      return 1;
    }
  }
  return 0;
}

void calc_new_position(state_t *state, double dt) {
  list_t *polygon = state->star;
  vector_t translation = vec_multiply(dt, state->velocity);
  polygon_translate(polygon, translation);
  polygon_rotate(polygon, omega, polygon_centroid(polygon));
  if (check_x_bounds(state)) {
    state->velocity = (vector_t){-1 * (state->velocity).x, (state->velocity).y};
  }
  if (check_y_bounds(state)) {
    state->velocity = (vector_t){(state->velocity).x, -1 * (state->velocity).y};
  }
}

void emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  list_t *polygon = state->star;
  sdl_clear();
  sdl_draw_polygon(polygon, color);
  state->star = polygon;
  sdl_show();
  calc_new_position(state, dt);
}

void emscripten_free(state_t *state) {
  list_free(state->star);
  free(state);
}