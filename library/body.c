#include "body.h"
#include "color.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>

struct body {
  rgb_color_t color;
  vector_t velocity;
  vector_t centroid;
  vector_t current_impulse;
  vector_t current_force;
  list_t *shape;
  void *info;
  free_func_t info_free;
  double angle;
  double mass;
  bool is_removed;
};

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  body_t *result = malloc(sizeof(body_t));
  result->info = info;
  result->info_free = info_freer;
  result->shape = shape;
  result->velocity = VEC_ZERO;
  result->mass = mass;
  result->color = color;
  result->centroid = polygon_centroid(shape);
  result->angle = 0;
  result->current_force = VEC_ZERO;
  result->current_impulse = VEC_ZERO;
  result->is_removed = false;
  return result;
}

void body_free(body_t *body) {
  list_free(body->shape);
  if (body->info_free != NULL) {
    (body->info_free)(body->info);
  }
  free(body);
}

void *body_get_info(body_t *body) { return body->info; }

list_t *body_get_shape(body_t *body) {
  list_t *shape = list_init(list_size(body->shape), free);
  for (size_t i = 0; i < list_size(body->shape); i++) {
    vector_t *vec = malloc(sizeof(vector_t));
    *vec = *((vector_t *)list_get(body->shape, i));
    list_add(shape, vec);
  }
  return shape;
}

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

rgb_color_t body_get_color(body_t *body) { return body->color; }

void body_set_centroid(body_t *body, vector_t x) {
  vector_t translation = vec_subtract(x, body->centroid);
  polygon_translate(body->shape, translation);
  body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->shape, angle - body->angle, body->centroid);
  body->angle = angle;
}

double body_get_mass(body_t *body) { return body->mass; }

void body_add_force(body_t *body, vector_t force) {
  body->current_force = vec_add(body->current_force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->current_impulse = vec_add(body->current_impulse, impulse);
}

void body_tick(body_t *body, double dt) {
  vector_t velocity_change =
      vec_add(vec_multiply(dt, body->current_force), body->current_impulse);
  velocity_change = vec_multiply(1 / body->mass, velocity_change);
  vector_t translation = vec_multiply(
      dt, vec_add(body->velocity, vec_multiply(0.5, velocity_change)));

  polygon_translate(body->shape, translation);
  body->velocity = vec_add(body->velocity, velocity_change);
  body->centroid = (vec_add(body->centroid, translation));
  body->current_force = VEC_ZERO;
  body->current_impulse = VEC_ZERO;
}

void body_remove(body_t *body) { body->is_removed = true; }

bool body_is_removed(body_t *body) { return body->is_removed; }

double body_get_rotation(body_t *body) { return body->angle; }

body_t *body_copy(body_t *body) {
  body_t *new_body = malloc(sizeof(body_t));
  *new_body = *body;
  new_body->shape = body_get_shape(body);
  new_body->info_free = NULL;
  return new_body;
}
