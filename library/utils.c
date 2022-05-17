#include "utils.h"
#include "list.h"

list_t *make_square() {
  list_t *shape = list_init(4, free);
  vector_t *v = malloc(sizeof(*v));
  *v = (vector_t){+1, +1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-1, +1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){-1, -1};
  list_add(shape, v);
  v = malloc(sizeof(*v));
  *v = (vector_t){+1, -1};
  list_add(shape, v);
  return shape;
}

double gen_rand(size_t lower, size_t upper) {
  double rand_num = (double)(rand() % (upper - lower + 1)) + lower;
  return rand_num;
}
rgb_color_t rand_color() {
  return (rgb_color_t){gen_rand(0, 255) / 255, gen_rand(0, 255) / 255,
                       gen_rand(0, 255) / 255};
}

vector_t rand_vec(size_t lower, size_t upper) {
  return (vector_t){gen_rand(lower, upper), gen_rand(lower, upper)};
}

void seed_random() { srand(time(0)); }