#include "body.h"
#include "color.h"
#include "list.h"
#include "polygon.h"
#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

struct body {
  vector_t velocity;
  vector_t centroid;
  vector_t current_impulse;
  vector_t current_force;
  rgb_color_t base_color;
  list_t *shapes;
  list_t *base_shape;
  void *info;
  free_func_t info_free;
  double angle;
  double mass;
  bool is_removed;
};

typedef struct shape_node {
  rgb_color_t color;
  list_t *shape;
} shape_node_t;

shape_node_t *node_init(list_t *shape, rgb_color_t color) {
  shape_node_t *new_node = malloc(sizeof(shape_node_t));
  new_node -> shape = shape;
  new_node -> color = color;
  return new_node;
}

void shape_node_free(shape_node_t *node) {
  list_free(node -> shape);
  free(node);
}

list_t *node_get_shape(shape_node_t *node) {
  return node -> shape;
}

rgb_color_t node_get_color(shape_node_t *node) {
  return node -> color;
}

rgb_color_t body_get_color(body_t *body) {
  return body -> base_color;
}

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  return body_init_with_info(shape, mass, color, NULL, NULL);
}

void body_add_shape(body_t *body, list_t *shape, rgb_color_t color) {
  shape_node_t *node= node_init(shape, color);
  list_add(body -> shapes, node);
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  body_t *result = malloc(sizeof(body_t));
  result->info = info;
  result->info_free = info_freer;
  result->shapes = list_init(5, (void *)shape_node_free);
  result -> base_shape = shape;
  assert(result -> base_shape != NULL);
  shape_node_t *node = node_init(shape, color);
  result -> base_color = color;
  list_add(result -> shapes, node);
  result->velocity = VEC_ZERO;
  result->mass = mass;
  result->centroid = polygon_centroid(result -> base_shape);
  result->angle = 0;
  result->current_force = VEC_ZERO;
  result->current_impulse = VEC_ZERO;
  result->is_removed = false;
  return result;
}

void body_free(body_t *body) {
  list_free(body->shapes);
  if (body->info_free != NULL) {
    (body->info_free)(body->info);
  }
  free(body);
}

void *body_get_info(body_t *body) { return body->info; }

list_t *vec_list_copy(list_t *list) {
  list_t *shape = list_init(list_size(list), free);
  for (size_t i = 0; i < list_size(list); i++) {
    vector_t *vec = malloc(sizeof(vector_t));
    *vec = *((vector_t *)list_get(list, i));
    list_add(shape, vec);
  }
  return shape;
}

shape_node_t *copy_node(shape_node_t *node) {
  shape_node_t *new_node = node_init(node -> shape, node -> color);
  return new_node;
}

list_t *body_get_base_shape(body_t *body) {
  if(body -> base_shape != NULL) {
    return vec_list_copy(body -> base_shape);
  }
  else return NULL;
}

list_t *body_get_shapes(body_t *body) {
  list_t *shapes = list_init(list_size(body -> shapes), (void *)list_free);
  for(size_t i = 0; i < list_size(body -> shapes); i++) {
    list_add(shapes, copy_node(list_get(body -> shapes, i)));
  }
  return shapes;
}

vector_t body_get_centroid(body_t *body) { return body->centroid; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

void body_set_centroid(body_t *body, vector_t x) {
  vector_t translation = vec_subtract(x, body->centroid);
  for(size_t i = 0; i < list_size(body -> shapes); i++) {
    polygon_translate(((shape_node_t *)list_get(body->shapes, i)) -> shape, translation);
  }
  body->centroid = x;
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_rotation(body_t *body, double angle) {
  for(size_t i = 0; i < list_size(body -> shapes); i++) {
  polygon_rotate(((shape_node_t *)list_get(body->shapes, i)) -> shape, angle - body->angle, body->centroid);
  }
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
  for(size_t i = 0; i < list_size(body -> shapes); i++) {
    polygon_translate(((shape_node_t *)list_get(body->shapes, i)) -> shape, translation);
  }
  body->velocity = vec_add(body->velocity, velocity_change);
  body->centroid = (vec_add(body->centroid, translation));
  body->current_force = VEC_ZERO;
  body->current_impulse = VEC_ZERO;
}

double body_get_angle(body_t *body) {
  return body -> angle;
}

void body_remove(body_t *body) { body->is_removed = true; }

bool body_is_removed(body_t *body) { return body->is_removed; }

double body_get_rotation(body_t *body) { return body->angle; }

body_t *body_copy(body_t *body) {
  body_t *new_body = malloc(sizeof(body_t));
  *new_body = *body;
  new_body->shapes = body_get_shapes(body);
  new_body -> base_shape = list_get(new_body -> shapes, 0);
  new_body->info_free = NULL;
  return new_body;
}
