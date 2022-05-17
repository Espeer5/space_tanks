#include "body.h"
#include "collision.h"
#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "utils.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
const size_t BRICK_COLUMNS = 10;
const double XMAX = 1200;
const double YMAX = 750;
const double BRICK_SPACING = 8;
const double BRICK_MASS = INFINITY;
const size_t BRICK_ROWS = 3;
const double TOTAL_BRICK_HEIGHT = 120;
const vector_t INITIAL_BALL_VELO = {400, 400};
const vector_t INITIAL_BALL_POSITION = {600, 50};
const double BALL_RADIUS = 20;
const double BALL_MASS = 20;
const double PLATFORM_MASS = INFINITY;
const rgb_color_t PLATFORM_COLOR = {.5, .5, .5};
const vector_t PLATFORM_INT_POSITION = {200, 20};
const double PLATFORM_WIDTH = 100;
const double PLATFORM_HEIGHT = 20;
const double PLATFORM_VELOCITY = 500;
const double OUTSIDE_WIDTH = 1;
const double OMEGA = 2;
const double POWER_RAND_LIM = 20000;
const double POWER_FREQ = 50; // smaller means less frequent powerups
const double PROJECTILE_MASS = 20;
const rgb_color_t PROJECTILE_COLOR = {0, 0, 1};
const double PROJECTILE_VELOCITY = 500;
const double PROJECTILE_WIDTH = 6;
const double PROJECTILE_OFFSET = 9;
const rgb_color_t POWERUP_COLOR = {0, 1, 0};
const double POWERUP_VELO = 100;
const size_t SHOTS_PER_POWER = 10;
const double POWERUP_DIMENSION = 15;
const double POWERUP_OFFSET = 20;
const double ELASTICITY = 1;
const size_t N_PRED_POINTS = 100;
const double PRED_DT = 0.01;
const double PRED_C_RAD = 10;
const double PRED_C_POINTS = 10;
const rgb_color_t PRED_COLOR = {0.2, 0.2, 0.2};

typedef struct state {
  size_t powered;
  scene_t *scene;
  size_t loops;
  double total_time;
} state_t;

void power_up_collision(body_t *body1, body_t *body2, vector_t axis,
                        void *aux) {
  ((state_t *)aux)->powered += SHOTS_PER_POWER;
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

void add_plat_projectile(vector_t base, state_t *state) {
  char *info = malloc(7 * sizeof(char));
  strcpy(info, "bullet");
  body_t *bod = body_init_with_info(projectile_init(base), PROJECTILE_MASS,
                                    PROJECTILE_COLOR, (void *)info, free);
  body_set_velocity(bod, (vector_t){0, PROJECTILE_VELOCITY});
  scene_add_body(state->scene, bod);
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    if (!strcmp((char *)body_get_info(scene_get_body(state->scene, i)),
                "brick")) {
      create_destructive_collision(state->scene, bod,
                                   scene_get_body(state->scene, i));
    }
  }
}

list_t *brick_init(vector_t bot_left, double brick_width, double brick_height) {
  list_t *brick = list_init(4, free);
  vector_t *first = malloc(sizeof(vector_t));
  *first = (vector_t){bot_left.x, bot_left.y};
  list_add(brick, first);
  vector_t *second = malloc(sizeof(vector_t));
  *second = (vector_t){bot_left.x, bot_left.y + brick_height};
  list_add(brick, second);
  vector_t *third = malloc(sizeof(vector_t));
  *third = (vector_t){bot_left.x + brick_width, bot_left.y + brick_height};
  list_add(brick, third);
  vector_t *fourth = malloc(sizeof(vector_t));
  *fourth = (vector_t){bot_left.x + brick_width, bot_left.y};
  list_add(brick, fourth);
  return brick;
}

void key_handle(char key, key_event_type_t type, double held_time,
                state_t *state) {
  if (type == KEY_PRESSED) {
    switch (key) {
    case LEFT_ARROW:
      body_set_velocity(scene_get_body(state->scene, 1),
                        (vector_t){-PLATFORM_VELOCITY, 0});
      break;
    case RIGHT_ARROW:
      body_set_velocity(scene_get_body(state->scene, 1),
                        (vector_t){PLATFORM_VELOCITY, 0});
      break;
    case SPACE:
      if (state->powered > 0) {
        state->powered -= 1;
        add_plat_projectile(
            vec_add(body_get_centroid(scene_get_body(state->scene, 1)),
                    (vector_t){0, PROJECTILE_OFFSET}),
            state);
      }
      break;
    }
  }
  if (type == KEY_RELEASED) {
    body_set_velocity(scene_get_body(state->scene, 1), (vector_t){0, 0});
  }
}

void create_brick_row(state_t *state, double col_height, double brick_width,
                      double brick_height, double brick_spacing) {
  for (size_t i = 0; i < BRICK_COLUMNS; i++) {
    char *info = malloc(6 * sizeof(char));
    strcpy(info, "brick");
    vector_t x =
        (vector_t){(i * brick_width) + ((i + 1) * brick_spacing), col_height};
    body_t *new_bod = body_init_with_info(
        brick_init(x, brick_width, brick_height), BRICK_MASS,
        (rgb_color_t){fabs(cosf(x.x * (M_PI / XMAX))),
                      fabs(sinf(x.x * (M_PI / XMAX))),
                      fabs(sinf(x.x * (M_PI / XMAX) + M_PI / 4))},
        info, free);
    scene_add_body(state->scene, new_bod);
  }
}

void create_bricks(state_t *state) {
  double brick_width =
      (XMAX - ((BRICK_COLUMNS + 1) * BRICK_SPACING)) / BRICK_COLUMNS;
  double brick_height =
      (TOTAL_BRICK_HEIGHT - ((BRICK_ROWS + 1) * BRICK_SPACING)) / BRICK_ROWS;
  for (size_t i = 0; i < BRICK_ROWS; i++) {
    create_brick_row(
        state, YMAX - (((i + 1) * brick_height) + ((i + 1) * (BRICK_SPACING))),
        brick_width, brick_height, BRICK_SPACING);
  }
}

void create_walls(state_t *state) {
  list_t *left = list_init(2, (free_func_t)free);
  list_t *right = list_init(2, (free_func_t)free);
  list_t *top = list_init(2, free);
  vector_t *vec = malloc(sizeof(vector_t));
  *vec = (vector_t){0, YMAX};
  list_add(top, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){XMAX, YMAX};
  list_add(top, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){XMAX, YMAX + OUTSIDE_WIDTH};
  list_add(top, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){0, YMAX + OUTSIDE_WIDTH};
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){0, 0};
  list_add(left, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){0, YMAX};
  list_add(left, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){-OUTSIDE_WIDTH, YMAX};
  list_add(left, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){-OUTSIDE_WIDTH, 0};
  list_add(left, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){XMAX, YMAX};
  list_add(right, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){XMAX, 0};
  list_add(right, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){XMAX + OUTSIDE_WIDTH, 0};
  list_add(right, vec);
  vec = malloc(sizeof(vector_t));
  *vec = (vector_t){XMAX + OUTSIDE_WIDTH, YMAX};
  list_add(right, vec);
  body_t *top_body = body_init(top, INFINITY, PLATFORM_COLOR);
  scene_add_body(state->scene, top_body);
  body_t *left_body = body_init(left, INFINITY, PLATFORM_COLOR);
  scene_add_body(state->scene, left_body);
  body_t *right_body = body_init(right, INFINITY, PLATFORM_COLOR);
  scene_add_body(state->scene, right_body);
}

void destroy_second(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  body_remove(body2);
}

double loop_position(double position, double min, double max) {
  if (position < min) {
    return position + (max - min);
  }
  if (position > max) {
    return position - (max - min);
  }
  return position;
}

void wrap_platform(state_t *state) {
  list_t *plat_shape = body_get_shape(scene_get_body(state->scene, 1));
  vector_t plat_centroid = (body_get_centroid(scene_get_body(state->scene, 1)));
  vector_t plat_position = *((vector_t *)list_get(plat_shape, 0));
  vector_t diff = vec_subtract(plat_centroid, plat_position);
  plat_position.x = loop_position(
      plat_position.x, 0,
      XMAX - (XMAX - ((BRICK_COLUMNS + 1) * BRICK_SPACING)) / BRICK_COLUMNS);
  body_set_centroid(scene_get_body(state->scene, 1),
                    vec_add(diff, plat_position));
  list_free(plat_shape);
}

state_t *emscripten_init() {
  seed_random();
  sdl_init(VEC_ZERO, (vector_t){XMAX, YMAX});
  state_t *state = malloc(sizeof(state_t));
  assert(state != NULL);
  state->scene = scene_init();
  state->powered = 0;
  state->loops = 0;
  state->total_time = 0;
  char *info = malloc(5 * sizeof(char));
  strcpy(info, "ball");
  scene_add_body(
      state->scene,
      body_init_with_info(star_init(5, INITIAL_BALL_POSITION, BALL_RADIUS),
                          BALL_MASS, rand_color(), info, free));
  char *info2 = malloc(9 * sizeof(char));
  strcpy(info2, "platform");
  scene_add_body(state->scene, body_init_with_info(
                                   brick_init(PLATFORM_INT_POSITION,
                                              PLATFORM_WIDTH, PLATFORM_HEIGHT),
                                   PLATFORM_MASS, PLATFORM_COLOR, info2, free));
  body_set_velocity(scene_get_body(state->scene, 0), INITIAL_BALL_VELO);
  create_walls(state);
  create_bricks(state);
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    create_physics_collision(state->scene, 1, scene_get_body(state->scene, 0),
                             scene_get_body(state->scene, i));
    if (i >= 5) {
      create_collision(state->scene, scene_get_body(state->scene, 0),
                       scene_get_body(state->scene, i),
                       (collision_handler_t)destroy_second, NULL, NULL);
    }
  }
  return state;
}

state_t *reset(state_t *state) {
  scene_free(state->scene);
  state->scene = scene_init();
  state->powered = 0;
  state->loops = 0;
  state->total_time = 0;
  char *info = malloc(5 * sizeof(char));
  strcpy(info, "ball");
  scene_add_body(
      state->scene,
      body_init_with_info(star_init(5, INITIAL_BALL_POSITION, BALL_RADIUS),
                          BALL_MASS, rand_color(), info, free));
  char *info2 = malloc(9 * sizeof(char));
  strcpy(info2, "platform");
  scene_add_body(state->scene, body_init_with_info(
                                   brick_init(PLATFORM_INT_POSITION,
                                              PLATFORM_WIDTH, PLATFORM_HEIGHT),
                                   PLATFORM_MASS, PLATFORM_COLOR, info2, free));
  body_set_velocity(scene_get_body(state->scene, 0), INITIAL_BALL_VELO);
  create_walls(state);
  create_bricks(state);
  for (size_t i = 1; i < scene_bodies(state->scene); i++) {
    create_physics_collision(state->scene, 1, scene_get_body(state->scene, 0),
                             scene_get_body(state->scene, i));
    if (i >= 5) {
      create_collision(state->scene, scene_get_body(state->scene, 0),
                       scene_get_body(state->scene, i),
                       (collision_handler_t)destroy_second, NULL, NULL);
    }
  }
  return state;
}

list_t *future_positions(state_t *state, double dt, size_t npoints) {
  list_t *pos = list_init(npoints, (free_func_t)free);
  list_t *prev_collided =
      list_init(scene_bodies(state->scene), (free_func_t)free);
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    bool *p = malloc(sizeof(bool));
    *p = false;
    list_add(prev_collided, p);
  }
  body_t *new_bod = body_copy(scene_get_body(state->scene, 0));
  for (size_t i = 0; i < npoints; i++) {
    body_tick(new_bod, dt);
    body_set_rotation(new_bod, OMEGA * (state->total_time + i * dt));
    for (size_t j = 1; j < scene_bodies(state->scene); j++) {
      list_t *b1 = body_get_shape(new_bod);
      body_t *body2 = scene_get_body(state->scene, j);
      list_t *b2 = body_get_shape(body2);
      collision_info_t info = find_collision(b1, b2);
      if (info.collided) {
        if (!*(bool *)list_get(prev_collided, j)) {
          vector_t impulse = get_impulse(new_bod, body2, info.axis, ELASTICITY);
          body_add_impulse(new_bod, impulse);
        }
        *(bool *)list_get(prev_collided, j) = true;
      } else {
        *(bool *)list_get(prev_collided, j) = false;
      }
      list_free(b1);
      list_free(b2);
    }
    vector_t *centroid = malloc(sizeof(vector_t));
    *centroid = body_get_centroid(new_bod);
    list_add(pos, (void *)centroid);
  }
  body_free(new_bod);
  return pos;
}

list_t *pred_circle(vector_t center) {
  list_t *result = list_init(PRED_C_POINTS, (free_func_t)free);
  for (size_t i = 0; i < PRED_C_POINTS; i++) {
    vector_t translation = (vector_t){cos(2 * i * M_PI / PRED_C_POINTS),
                                      sin(2 * i * M_PI / PRED_C_POINTS)};
    vector_t *point = malloc(sizeof(vector_t));
    *point = vec_add(center, translation);
    list_add(result, point);
  }
  return result;
}

void add_future_positions(state_t *state, double dt, size_t npoints) {
  list_t *pos = future_positions(state, dt, npoints);
  for (size_t i = 0; i < npoints; i++) {
    vector_t center = *(vector_t *)list_get(pos, i);
    body_t *new_body = body_init(pred_circle(center), 1, PRED_COLOR);
    scene_add_body(state->scene, new_body);
    scene_remove_body(state->scene, scene_bodies(state->scene) - 1);
  }
  list_free(pos);
}

void emscripten_main(state_t *state) {
  wrap_platform(state);
  sdl_on_key(key_handle);
  state->loops += 1;
  double dt = time_since_last_tick();
  state->total_time += dt;
  if (gen_rand(0, POWER_RAND_LIM) < POWER_FREQ) {
    body_t *powerup = body_init(
        brick_init(
            (vector_t){gen_rand(POWERUP_OFFSET, XMAX - POWERUP_OFFSET), YMAX},
            POWERUP_DIMENSION, POWERUP_DIMENSION),
        BRICK_MASS, POWERUP_COLOR);
    body_set_velocity(powerup, (vector_t){0, -POWERUP_VELO});
    scene_add_body(state->scene, powerup);
    create_collision(state->scene, scene_get_body(state->scene, 1), powerup,
                     (collision_handler_t)destroy_second, NULL, NULL);
    create_collision(state->scene, powerup, scene_get_body(state->scene, 1),
                     (collision_handler_t)power_up_collision, state, NULL);
  }
  body_set_rotation(scene_get_body(state->scene, 0),
                    (OMEGA * (state->total_time)));

  add_future_positions(state, PRED_DT, N_PRED_POINTS);
  sdl_render_scene(state->scene);
  scene_tick(state->scene, dt);
  if ((body_get_centroid(scene_get_body(state->scene, 0))).y - BALL_RADIUS <=
      0) {
    reset(state);
  }
}

void emscripten_free(state_t *state) {
  scene_free(state->scene);
  free(state);
}
