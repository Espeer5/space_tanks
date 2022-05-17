#include "list.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

void test_size0() {
  list_t *list = list_init(0, free);
  assert(list_size(list) == 0);
  list_free(list);
}

void test_str_size1() {
  list_t *list = list_init(1, free);
  assert(list_size(list) == 0);
  char *str = malloc(sizeof(char));
  add_at(list, str, 0);
  assert(list_size(list) == 1);
  assert(list_remove(list, 0) == str);
  assert(list_size(list) == 0);
  char *str2 = malloc(sizeof(char));
  list_add(list, str2);
  assert(list_size(list) == 1);
  assert(dequeue(list) == str2);
  assert(list_size(list) == 0);
  list_free(list);
}

void illegal_get_access(list_t *list) { list_get(list, 6); }

void illegal_add_access(list_t *list) {
  char *new = malloc(sizeof(char *));
  add_at(list, new, 6);
}

void illegal_remove_access(list_t *list) { list_remove(list, 6); }

void test_illegal_access() {
  list_t *list = list_init(1, free);
  for (size_t i = 0; i <= 4; i++) {
    char *str = malloc(sizeof(char *));
    list_add(list, str);
  }
  assert(test_assert_fail((void *)illegal_get_access, list));
  assert(test_assert_fail((void *)illegal_add_access, list));
  assert(test_assert_fail((void *)illegal_remove_access, list));
  list_free(list);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_size0)
  DO_TEST(test_str_size1)
  DO_TEST(test_illegal_access)

  puts("list_test PASS");
}
