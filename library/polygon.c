#include "polygon.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double polygon_area(list_t *polygon) {
  double area = 0;
  vector_t *prev = (vector_t *)list_get(polygon, list_size(polygon) - 1);
  for (size_t i = 0; i < list_size(polygon); i++) {
    area = area + vec_cross(*prev, *((vector_t *)list_get(polygon, i)));
    prev = ((vector_t *)list_get(polygon, i));
  }
  return area / 2;
}

vector_t polygon_centroid(list_t *polygon) {
  double coef = 1 / (6 * polygon_area(polygon));
  double y_comp = 0;
  double x_comp = 0;
  vector_t *prev = (vector_t *)list_get(polygon, list_size(polygon) - 1);
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t *v2 = ((vector_t *)list_get(polygon, i));
    vector_t *v1 = prev;
    double cross = vec_cross(*v1, *v2);
    y_comp = y_comp + (v1->y + v2->y) * cross;
    x_comp = x_comp + (v1->x + v2->x) * cross;
    prev = v2;
  }
  x_comp = x_comp * coef;
  y_comp = y_comp * coef;
  vector_t to_return = {x_comp, y_comp};
  return to_return;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    *((vector_t *)list_get(polygon, i)) =
        vec_add(*((vector_t *)list_get(polygon, i)), translation);
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t vertex = *((vector_t *)list_get(polygon, i));
    vector_t offset = vec_subtract(vertex, point);
    *((vector_t *)list_get(polygon, i)) =
        vec_add(vec_rotate(offset, angle), point);
  }
}

list_t *star_init(size_t number_points, vector_t center, double star_radius) {
  double d_angle = (M_PI / number_points);
  double total_angle = 0;
  size_t counter = 2;
  double scaler = sin(M_PI / (2 * number_points)) /
                  sin(((number_points - 3) * M_PI) / number_points);
  list_t *star = list_init(2 * number_points, free);
  for (size_t i = 0; i < 2 * number_points; i++) {
    total_angle = total_angle + d_angle;
    vector_t *new_vec_pointer = malloc(sizeof(vector_t));
    assert(new_vec_pointer != NULL);
    if (counter % 2) {
      *new_vec_pointer =
          (vector_t){center.x - (star_radius * scaler * cos(total_angle)),
                     center.y - (star_radius * scaler * sin(total_angle))};

    } else {
      *new_vec_pointer =
          (vector_t){center.x - (star_radius * cos(total_angle)),
                     center.y - (star_radius * sin(total_angle))};
    }
    list_add(star, new_vec_pointer);
    counter++;
  }
  return star;
}