#include "list.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const size_t GROW_FACTOR = 2;

typedef struct list {
  size_t size;
  size_t capacity;
  void **array;
  free_func_t freer;
} list_t;

typedef void (*free_funct_t)(void *object);

typedef void* (*copy_funct_t)(void *object);

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *new_list = malloc(sizeof(list_t));
  assert(new_list != NULL);
  assert(new_list->array != NULL);
  new_list->size = 0;
  if (initial_size != 0) {
    new_list->capacity = initial_size;
    new_list->array = malloc(initial_size * sizeof(void *));
  } else {
    new_list->capacity = 1;
    new_list->array = malloc(sizeof(void *));
  }
  new_list->freer = freer;
  return new_list;
}

bool ensure_size(list_t *arr, size_t num_elem) {
  if (arr->capacity == 0) {
    void **new_data = malloc(5 * sizeof(void *));
    assert(new_data != NULL);
    free(arr->array);
    arr->array = new_data;
    return 1;
  } else if (arr->size + num_elem <= arr->capacity) {
    return 0;
  } else {
    void **new_data = malloc((GROW_FACTOR * arr->capacity) * sizeof(void *));
    assert(new_data != NULL);
    for (size_t i = 0; i < arr->size; i++) {
      new_data[i] = (arr->array)[i];
    }
    free(arr->array);
    arr->array = new_data;
    arr->capacity = GROW_FACTOR * arr->capacity;
    return 1;
  }
}

void *list_get(list_t *list, size_t idx) {
  assert(0 <= idx && idx < list->size);
  return (list->array)[idx];
}

void add_at(list_t *list, void *object, size_t idx) {
  assert(0 <= idx && idx <= (list->size));
  ensure_size(list, 1);
  for (size_t i = list->size; i > idx; i--) {
    (list->array)[i] = (list->array)[i - 1];
  }
  (list->array)[idx] = object;
  list->size += 1;
}

void *list_remove(list_t *list, size_t index) {
  assert(0 <= index && index < list->size);
  void *to_return = list_get(list, index);
  if (list->freer != NULL && list->array[index] != NULL) {
    list->freer(list->array[index]);
  }
  for (size_t i = index; i < list->size - 1; i++) {
    (list->array)[i] = (list->array)[i + 1];
  }
  (list->size)--;
  return to_return;
}

void list_add(list_t *list, void *object) { add_at(list, object, list->size); }

void *dequeue(list_t *list) { return list_remove(list, 0); }

void list_free(list_t *list) {
  if (list != NULL) {
    if (list->array != NULL) {
      for (size_t i = 0; i < list->size; i++) {
        if (list->array[i] != NULL && list->freer != NULL) {
          list->freer(list->array[i]);
        }
      }
      free(list->array);
    }
    free(list);
  }
}

size_t list_size(list_t *list) { return list->size; }

size_t get_capacity(list_t *list) { return list->capacity; }
/*
list_t *list_copy(list_t *list, copy_func_t copier) {
  list_t *new_list = list_init(list_size(list), list->freer);
  for (size_t i = 0; i < list_size(list); i++) {
    list_add(new_list, copier(list_get(list, i)));
  }
  return new_list;
}*/

list_t *list_merge(list_t *list1, list_t *list2, copy_func_t copier) {
  assert(list1->freer == list2->freer); //otherwise this makes no sense
  list_t *new_list = list_init(list_size(list1) + list_size(list2), list1->freer);
  for (size_t i = 0; i < list_size(list1); i++) {
    list_add(new_list, copier(list_get(list1, i)));
  }
  for (size_t i = 0; i < list_size(list2); i++) {
    list_add(new_list, copier(list_get(list2, i)));
  }
  return new_list;
}