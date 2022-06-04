#include "list.h"
#include "vector.h"
#include <math.h>
#include <stdbool.h>
#include "polygon.h"

/**
 * Determines whether two convex polygons intersect.
 * The polygons are given as lists of vertices in counterclockwise order.
 * There is an edge between each pair of consecutive vertices,
 * and one between the first vertex and the last vertex.
 *
 * @param shape1 the first shape
 * @param shape2 the second shape
 * @return whether the shapes are colliding
 */

vector_t make_axis(vector_t vector) {
  return vec_multiply(1 / sqrt(vec_dot(vector, vector)),
                      vec_rotate(vector, M_PI / 2));
}

vector_t make_projection(vector_t vector, list_t *shape) {
  double max = vec_dot(vector, *(vector_t *)list_get(shape, 0));
  double min = max;
  for (size_t i = 1; i < list_size(shape); i++) {
    double dot_product = vec_dot(vector, *(vector_t *)list_get(shape, i));
    if (dot_product > max) {
      max = dot_product;
    }
    if (dot_product < min) {
      min = dot_product;
    }
  }
  vector_t projection = {min, max};
  return projection;
}
double overlap_magnitude(double min1, double max1, double min2, double max2){
  double overlap = 0;
  if(min2 > min1 && max2 > max1 && max1 > min2){
    overlap = max1 - min2;
  }
  if(min1 > min2 && max1 > max2 && max2 > min1) {
    overlap = max2 - min1;
  }
  if(min1 < min2 && max1 > max2){
    overlap = max2 - min2;
  }
  if(min2 < min1 && max2 > max1){
    overlap = max1 - min1;
  }
  return overlap;
}
typedef struct {
    bool collided;
    vector_t axis;
} collision_info_t;

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  size_t axis1 = list_size(shape1);
  size_t axis2 = list_size(shape2);
  collision_info_t least_overlap = (collision_info_t){false, VEC_ZERO};
  double min_overlap = INFINITY;
  for (size_t i = 0; i < axis1; i++) {
    vector_t vector =
        vec_subtract(*(vector_t *)list_get(shape1, i),
                     *(vector_t *)list_get(shape1, (i + 1) % axis1));
    vector_t perpendicular = make_axis(vector);
    vector_t projection1 = make_projection(perpendicular, shape1);
    vector_t projection2 = make_projection(perpendicular, shape2);
    double min1 = projection1.x;
    double max1 = projection1.y;
    double min2 = projection2.x;
    double max2 = projection2.y;
    if (overlap_magnitude(min1, max1, min2, max2) < min_overlap){
      min_overlap = overlap_magnitude(min1, max1, min2, max2);
      if(min_overlap > 0) {
        least_overlap = (collision_info_t){true, perpendicular};
      }
      else {
        least_overlap = (collision_info_t){false, VEC_ZERO};
      }
    }

  }
  for (size_t i = 0; i < axis2; i++) {
    vector_t vector =
        vec_subtract(*(vector_t *)list_get(shape2, i),
                     *(vector_t *)list_get(shape2, (i + 1) % axis2));
    vector_t perpendicular = make_axis(vector);
    vector_t projection1 = make_projection(perpendicular, shape1);
    vector_t projection2 = make_projection(perpendicular, shape2);
    double min1 = projection1.x;
    double max1 = projection1.y;
    double min2 = projection2.x;
    double max2 = projection2.y;
    if (overlap_magnitude(min1, max1, min2, max2) < min_overlap){
      min_overlap = overlap_magnitude(min1, max1, min2, max2);
      if(min_overlap > 0) {
        least_overlap = (collision_info_t){true, perpendicular};
      }
      else {
        least_overlap = (collision_info_t){false, VEC_ZERO};
      }
    }
  }
  vector_t body1 = polygon_centroid(shape1);
  vector_t body2 = polygon_centroid(shape2);
  if (vec_dot(vec_subtract(body2, body1), least_overlap.axis) < 0) {
    least_overlap.axis = vec_negate(least_overlap.axis);
  }
  return least_overlap;
}
