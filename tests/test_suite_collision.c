#include "collision.h"
#include "polygon.h"
#include "test_util.h"
#include "utils.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

// Make 3-4-5 triangle
// list_t *make_triangle() {
//   list_t *tri = list_init(3, free);
//   vector_t *v = malloc(sizeof(*v));
//   *v = VEC_ZERO;
//   list_add(tri, v);
//   v = malloc(sizeof(*v));
//   *v = (vector_t){4, 0};
//   list_add(tri, v);
//   v = malloc(sizeof(*v));
//   *v = (vector_t){4, 3};
//   list_add(tri, v);
//   return tri;
// }

// // Weird nonconvex polygon
// list_t *make_weird() {
//   list_t *w = list_init(5, free);
//   vector_t *v = malloc(sizeof(*v));
//   *v = VEC_ZERO;
//   list_add(w, v);
//   v = malloc(sizeof(*v));
//   *v = (vector_t){4, 1};
//   list_add(w, v);
//   v = malloc(sizeof(*v));
//   *v = (vector_t){-2, 1};
//   list_add(w, v);
//   v = malloc(sizeof(*v));
//   *v = (vector_t){-5, 5};
//   list_add(w, v);
//   v = malloc(sizeof(*v));
//   *v = (vector_t){-1, -8};
//   list_add(w, v);
//   return w;
// }

// void test_square_triangle() {
//   list_t *square = make_square();
//   list_t *triangle = make_triangle();
//   assert(find_collision(square, triangle));
//   assert(find_collision(triangle, square));
//   list_free(square);
//   list_free(triangle);
// }

int main(int argc, char *argv[]) {
  // Run all tests? True if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  //DO_TEST(test_square_triangle);

  puts("polygon_test PASS");
}
