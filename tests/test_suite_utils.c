#include "test_util.h"
#include "utils.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

const size_t TEST_CYCLES = 200;

void test_gen_rand() {
  size_t i = 0;
  while (i < TEST_CYCLES) {
    double rand = gen_rand(0, 50);
    assert(rand >= 0 && rand <= 50);
    i++;
  }
}

void test_rand_color() {
  size_t i = 0;
  while (i < TEST_CYCLES) {
    rgb_color_t color = rand_color();
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);
    i++;
  }
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_gen_rand);
  DO_TEST(test_rand_color);

  puts("utils_test PASS");
}