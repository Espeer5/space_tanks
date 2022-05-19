#include "background.h"
#include <stdlib.h>

void background_obj_free(background_obj_t *obj) {
  list_free(obj -> polygon);
}

background_obj_t *obj_init(list_t *polygon, rgb_color_t color) {
  background_obj_t *new_obj = malloc(sizeof(background_obj_t));
  new_obj -> polygon = polygon;
  new_obj -> color = color;
  return new_obj;
}
