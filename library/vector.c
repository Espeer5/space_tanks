#include "vector.h"
#include <math.h>
#include <stdlib.h>

const vector_t VEC_ZERO = {0, 0};

vector_t vec_add(vector_t v1, vector_t v2) {
  return (vector_t){v1.x + v2.x, v1.y + v2.y};
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  return vec_add(v1, vec_negate(v2));
}

vector_t vec_negate(vector_t v) { return vec_multiply(-1, v); }

vector_t vec_multiply(double scaler, vector_t v) {
  return (vector_t){scaler * v.x, scaler * v.y};
}

double vec_dot(vector_t v1, vector_t v2) {
  return (v1.x * v2.x) + (v1.y * v2.y);
}

double vec_cross(vector_t v1, vector_t v2) {
  return (v1.x * v2.y) - (v2.x * v1.y);
}

vector_t vec_rotate(vector_t v, double angle) {
  return (vector_t){v.x * cos(angle) - v.y * sin(angle),
                    v.x * sin(angle) + v.y * cos(angle)};
}

double norm(vector_t vec) {
  return sqrt(vec_dot(vec, vec));
}