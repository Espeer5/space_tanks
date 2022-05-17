#include "body.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const double XMAX = 1000;
const double YMAX = 500;
const double STAR_RDIUS = 40;
const size_t INITIAL_POINTS = 5;
const vector_t INIT_POS = {0, 500};
const size_t CYCLES_BTW = 100;
const vector_t INT_VELO = {145, 0};
const vector_t END_OF_LINE = {1000, 0};
const vector_t G = {0, -100};

typedef struct state {
  size_t counter;
  size_t loops;
  list_t *shapes;
} state_t;

void y_collision(body_t *shape) {
  body_set_velocity(
      shape, (vector_t){body_get_velocity(shape).x,
                        -1 * ((float)rand() * (3 / 2) / (float)RAND_MAX) *
                            body_get_velocity(shape).y});
}

bool check_x(body_t *shape, double XMAX) {
  double x_min = __DBL_MAX__;
  list_t *body_shape = body_get_shape(shape);
  for (size_t i = 0; i < list_size(body_shape); i++) {
    double x_comp = (*(vector_t *)list_get(body_shape, i)).x;
    if (x_comp < x_min) {
      x_min = x_comp;
    };
  }
  list_free(body_shape);
  if (x_min > XMAX) {
    return true;
  } else {
    return false;
  }
}

bool check_y(body_t *shape) {
  double y_min = __DBL_MAX__;
  list_t *body_shape = body_get_shape(shape);
  for (size_t i = 0; i < list_size(body_shape); i++) {
    double y_comp = (*(vector_t *)list_get(body_shape, i)).y;
    if (y_comp < y_min) {
      y_min = y_comp;
    }
  }
  list_free(body_shape);
  if (y_min <= 0 && body_get_velocity(shape).y < 0) {
    return true;
  } else {
    return false;
  }
}

void gravity(body_t *body, double dt) {
  body_set_velocity(body,
                    vec_add(body_get_velocity(body), vec_multiply(dt, G)));
}

void advance_time(state_t *state, double dt) {
  for (size_t i = 0; i < list_size(state->shapes); i++) {
    list_t *shape = body_get_shape(list_get(state->shapes, i));
    sdl_draw_polygon(shape, body_get_color(list_get(state->shapes, i)));
    gravity(list_get(state->shapes, i), dt);
    body_tick(list_get(state->shapes, i), dt);
    if (check_y(list_get(state->shapes, i))) {
      y_collision(list_get(state->shapes, i));
    }
    list_free(shape);
  }
  if (check_x(list_get(state->shapes, list_size(state->shapes) - 1), XMAX)) {
    dequeue(state->shapes);
  }
}

state_t *emscripten_init() {
  vector_t min = (vector_t){0, 0};
  vector_t max = (vector_t){XMAX, YMAX};
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->shapes = list_init(3, (void *)body_free);
  state->counter = 0;
  state->loops = 0;
  list_t *first_star = star_init(INITIAL_POINTS, INIT_POS, 40);
  body_t *star_1 = body_init(first_star, 1, rand_color());
  body_set_velocity(star_1, INT_VELO);
  body_set_rotation(star_1, gen_rand(0, 1));
  list_add(state->shapes, star_1);
  return state;
}

void emscripten_main(state_t *state) {
  state->loops += 1;
  double dt = time_since_last_tick();
  sdl_clear();
  advance_time(state, dt);
  sdl_show();
  if (!(state->loops % CYCLES_BTW) && state->loops != 0) {
    state->counter += 1;
    list_t *new_star =
        star_init(INITIAL_POINTS + (state->counter), INIT_POS, STAR_RDIUS);
    body_t *star = body_init(new_star, 1, rand_color());
    body_set_velocity(star, INT_VELO);
    body_set_rotation(star, gen_rand(0, 1));
    list_add(state->shapes, star);
  }
}

void emscripten_free(state_t *state) {
  list_free(state->shapes);
  free(state);
}
