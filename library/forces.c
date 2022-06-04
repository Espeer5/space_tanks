#include "body.h"
#include "collision.h"
#include "scene.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

typedef void (*collision_handler_t)(body_t *body1, body_t *body2, vector_t axis,
                                    void *aux);

typedef struct auxiliary_data {
  list_t *constants;
  list_t *bodies;
} aux_t;

aux_t *auxiliary_data_init() {
  aux_t *data = malloc(sizeof(aux_t));
  assert(data);
  data->constants = list_init(1, free);
  assert(data->constants);
  data->bodies = list_init(2, NULL);
  assert(data->bodies);
  return data;
}

void free_auxiliary(void *aux) {
  aux_t *data = (aux_t *)aux;
  list_free(data->constants);
  list_free(data->bodies);
  free(data);
}

list_t *list_copy(list_t *list) {
  size_t size = list_size(list);
  list_t *new_list = list_init(size, NULL);
  for (size_t i = 0; i < size; i++) {
    list_add(new_list, list_get(list, i));
  }
  return new_list;
}

void newtonian_gravity(void *aux) {
  aux_t *data = (aux_t *)aux;
  double G = *((double *)list_get(data->constants, 0));
  body_t *body1 = ((body_t *)list_get(data->bodies, 0));
  body_t *body2 = ((body_t *)list_get(data->bodies, 1));
  vector_t gap =
      vec_subtract(body_get_centroid(body1), body_get_centroid(body2));
  double radius = sqrt(vec_dot(gap, gap));
  if (radius < 5) {
    return;
  }
  double numerator = -G * body_get_mass(body1) * body_get_mass(body2);
  vector_t force_on_1 =
      vec_multiply(numerator / (radius * radius * radius), gap);
  body_add_force(body1, force_on_1);
  body_add_force(body2, vec_negate(force_on_1));
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  double *g = malloc(sizeof(double));
  assert(g);
  *g = G;
  aux_t *data = auxiliary_data_init();
  list_add(data->bodies, body1);
  list_add(data->bodies, body2);
  list_add(data->constants, g);
  void *aux = (void *)data;
  free_func_t freer = (free_func_t)free_auxiliary;
  force_creator_t forcer = (force_creator_t)newtonian_gravity;
  scene_add_bodies_force_creator(scene, forcer, aux, list_copy(data->bodies),
                                 freer);
}

void spring_force(void *aux) {
  aux_t *data = (aux_t *)aux;
  double k = *((double *)list_get(data->constants, 0));
  body_t *body1 = ((body_t *)list_get(data->bodies, 0));
  body_t *body2 = ((body_t *)list_get(data->bodies, 1));
  vector_t gap =
      vec_subtract(body_get_centroid(body1), body_get_centroid(body2));
  vector_t force_on_1 = vec_multiply(-k, gap);
  body_add_force(body1, force_on_1);
  body_add_force(body2, vec_negate(force_on_1));
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  double *kp = malloc(sizeof(double));
  assert(kp);
  *kp = k;
  aux_t *data = auxiliary_data_init();
  list_add(data->bodies, body1);
  list_add(data->bodies, body2);
  list_add(data->constants, kp);
  void *aux = (void *)data;
  free_func_t freer = (free_func_t)free_auxiliary;
  force_creator_t forcer = (force_creator_t)spring_force;
  scene_add_bodies_force_creator(scene, forcer, aux, list_copy(data->bodies),
                                 freer);
}

void drag_force(void *aux) {
  aux_t *data = (aux_t *)aux;
  double gamma = *((double *)list_get(data->constants, 0));
  body_t *body = ((body_t *)list_get(data->bodies, 0));
  vector_t force = vec_multiply(-gamma, body_get_velocity(body));
  body_add_force(body, force);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  double *g = malloc(sizeof(double));
  assert(g);
  *g = gamma;
  aux_t *data = auxiliary_data_init();
  list_add(data->bodies, body);
  list_add(data->constants, g);
  void *aux = (void *)data;
  free_func_t freer = (free_func_t)free_auxiliary;
  force_creator_t forcer = (force_creator_t)drag_force;
  scene_add_bodies_force_creator(scene, forcer, aux, list_copy(data->bodies),
                                 freer);
}

typedef struct collision_aux {
  body_t *body1;
  body_t *body2;
  collision_handler_t handler;
  void *aux;
  free_func_t freer;
  size_t counter;
} collision_aux_t;

void free_collision_aux(void *aux) {
  collision_aux_t *data = (collision_aux_t *)aux;
  if (data->freer != NULL) {
    data->freer(data->aux);
  }
  free(data);
}

void collision_creator(void *aux) {
  collision_aux_t *data = (collision_aux_t *)aux;
  list_t *shape1 = body_get_base_shape(data->body1);
  list_t *shape2 = body_get_base_shape(data->body2);
  collision_info_t collision_axis = find_collision(shape1, shape2);
  if (collision_axis.collided) {
    if (data->counter < 1) {
      data->handler(data->body1, data->body2, collision_axis.axis, data->aux);
      data->counter++;
    }
  } else {
    data->counter = 0;
  }
  list_free(shape1);
  list_free(shape2);
}

/**
 * Adds a force creator to a scene that calls a given collision handler
 * function each time two bodies collide.
 * This generalizes create_destructive_collision() from last week,
 * allowing different things to happen on a collision.
 * The handler is passed the bodies, the collision axis, and an auxiliary value.
 * It should only be called once while the bodies are still colliding.
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 * @param handler a function to call whenever the bodies collide
 * @param aux an auxiliary value to pass to the handler
 * @param freer if non-NULL, a function to call in order to free aux
 */
void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  collision_aux_t *data = malloc(sizeof(collision_aux_t));
  data->body1 = body1;
  data->body2 = body2;
  data->handler = handler;
  data->aux = aux;
  data->freer = freer;
  data->counter = 0;
  list_t *bodies = list_init(2, NULL);
  list_add(bodies, body1);
  list_add(bodies, body2);
  void *new_aux = (void *)data;
  free_func_t new_freer = (free_func_t)free_collision_aux;
  force_creator_t forcer = (force_creator_t)collision_creator;
  scene_add_bodies_force_creator(scene, forcer, new_aux, bodies, new_freer);
}
void body_destroy(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  body_remove(body1);
  body_remove(body2);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  collision_handler_t handler = (collision_handler_t)body_destroy;
  void *aux = NULL;
  free_func_t freer = NULL;
  create_collision(scene, body1, body2, handler, aux, freer);
}

vector_t get_impulse(body_t *body1, body_t *body2, vector_t axis,
                     double elasticity) {
  double ma = body_get_mass(body1);
  double mb = body_get_mass(body2);
  double reduced_mass = 0;
  double ub = vec_dot(body_get_velocity(body2), axis);
  double ua = vec_dot(body_get_velocity(body1), axis);
  double jn = 0;
  vector_t impulse;
  if (ma == INFINITY) {
    reduced_mass = mb;
  }
  if (mb == INFINITY) {
    reduced_mass = ma;
  }
  if (reduced_mass == 0) {
    reduced_mass = ((ma * mb) / (ma + mb));
  }
  jn = (reduced_mass) * (1 + elasticity) * (ub - ua);
  impulse = vec_multiply(jn, axis);
  return impulse;
}

void applied_impulse(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  double elasticity = *(double *)aux;
  vector_t impulse = get_impulse(body1, body2, axis, elasticity);
  body_add_impulse(body1, impulse);
  body_add_impulse(body2, vec_negate(impulse));
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  collision_handler_t handler = (collision_handler_t)applied_impulse;
  double *not_aux = malloc(sizeof(elasticity));
  *not_aux = elasticity;
  void *aux = (void *)not_aux;
  free_func_t freer = (free_func_t)free;
  create_collision(scene, body1, body2, handler, aux, freer);
}
