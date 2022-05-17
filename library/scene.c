#include "scene.h"
#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct scene {
  list_t *bodies;
  list_t *force_creators;
  list_t *auxes;
  list_t *aux_freers;
  list_t *body_lists;
} scene_t;

typedef void (*force_creator_t)(void *aux);

scene_t *scene_init(void) {
  scene_t *new_scene = malloc(sizeof(scene_t));
  assert(new_scene != NULL);
  new_scene->bodies = list_init(5, (void *)body_free);
  assert(new_scene->bodies != NULL);
  new_scene->force_creators = list_init(5, NULL);
  assert(new_scene->force_creators != NULL);
  new_scene->auxes = list_init(5, NULL);
  assert(new_scene->auxes != NULL);
  new_scene->aux_freers = list_init(5, NULL);
  assert(new_scene->aux_freers != NULL);
  new_scene->body_lists = list_init(5, (void *)list_free);
  assert(new_scene->body_lists != NULL);
  return new_scene;
}

void free_auxes(scene_t *scene) {
  size_t size = list_size(scene->auxes);
  free_func_t freer;
  for (size_t i = 0; i < size; i++) {
    freer = (free_func_t)list_get(scene->aux_freers, i);
    if (freer != NULL) {
      if (list_get(scene->auxes, i) != NULL) {
        freer(list_get(scene->auxes, i));
      }
    }
  }
  list_free(scene->auxes);
  list_free(scene->aux_freers);
}

void scene_free(scene_t *scene) {
  if (scene->force_creators != NULL) {
    list_free(scene->force_creators);
  }
  free_auxes(scene);
  if (scene->bodies != NULL) {
    list_free(scene->bodies);
  }
  if (scene->body_lists != NULL) {
    list_free(scene->body_lists);
  }
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  void *to_ret = list_get(scene->bodies, index);
  return to_ret;
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  body_remove(scene_get_body(scene, index));
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  scene_add_bodies_force_creator(scene, forcer, aux, list_init(0, NULL), freer);
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  list_add(scene->force_creators, forcer);
  list_add(scene->auxes, aux);
  list_add(scene->aux_freers, freer);
  list_add(scene->body_lists, bodies);
}

void scene_tick(scene_t *scene, double dt) {
  for (size_t i = 0; i < list_size(scene->force_creators); i++) {
    force_creator_t forcer = list_get(scene->force_creators, i);
    void *aux = list_get(scene->auxes, i);
    forcer(aux);
  }
  for (size_t j = 0; j < list_size(scene->body_lists); j++) {
    for (size_t k = 0; k < list_size(list_get(scene->body_lists, j)); k++) {
      if (body_is_removed(list_get(list_get(scene->body_lists, j), k))) {
        list_remove(scene->body_lists, j);
        list_remove(scene->force_creators, j);
        free_func_t freer = (free_func_t)list_get(scene->aux_freers, j);
        if (freer != NULL) {
          freer(list_get(scene->auxes, j));
        }
        list_remove(scene->aux_freers, j);
        list_remove(scene->auxes, j);
        j--;
        break;
      }
    }
  }
  for (size_t l = 0; l < scene_bodies(scene); l++) {
    if (body_is_removed(scene_get_body(scene, l))) {
      list_remove(scene->bodies, l);
      l--;
    }
  }
  for (size_t t = 0; t < scene_bodies(scene); t++) {
    body_tick(scene_get_body(scene, t), dt);
  }
}