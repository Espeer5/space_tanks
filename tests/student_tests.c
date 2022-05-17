#include "forces.h"
#include "polygon.h"
#include "scene.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

list_t *new_shape(size_t number_points, vector_t center, double star_radius) {
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

double spring_potential(body_t *bod1, body_t *bod2, double k) {
  vector_t diff =
      vec_subtract(body_get_centroid(bod2), body_get_centroid(bod1));
  return .5 * k * sqrt(pow(vec_dot(diff, diff), 2));
}
double kinetic_energy(body_t *body) {
  vector_t v = body_get_velocity(body);
  return body_get_mass(body) * vec_dot(v, v) / 2;
}

void spring_conservation() {
  body_t *bod1 =
      body_init(new_shape(5, (vector_t){-5, 0}, 2), 5, (rgb_color_t){1, 1, 1});
  body_t *bod2 =
      body_init(new_shape(5, (vector_t){5, 0}, 2), 5, (rgb_color_t){1, 1, 1});
  size_t steps = 100;
  scene_t *scene = scene_init();
  scene_add_body(scene, bod1);
  scene_add_body(scene, bod2);
  create_spring(scene, 2, bod1, bod2);
  double init_energy = spring_potential(bod1, bod2, 2);
  for (size_t i = 0; i < steps; i++) {
    assert(body_get_centroid(bod1).x < body_get_centroid(bod2).x);
    double energy = spring_potential(bod1, bod2, 2) + kinetic_energy(bod1) +
                    kinetic_energy(bod2);
    assert(within(1e-4, energy / init_energy, 1));
    scene_tick(scene, .0001);
  }
  scene_free(scene);
}

void drag_spring_dampen_test() {
  body_t *bod1 =
      body_init(new_shape(5, (vector_t){-5, 0}, 2), 5, (rgb_color_t){1, 1, 1});
  body_t *bod2 =
      body_init(new_shape(5, (vector_t){5, 0}, 2), 5, (rgb_color_t){1, 1, 1});
  scene_t *scene = scene_init();
  scene_add_body(scene, bod1);
  scene_add_body(scene, bod2);
  create_spring(scene, 2, bod1, bod2);
  double y_max = 0;
  double predicted_A;
  for (size_t i = 0; i < 1000; i++) {
    scene_tick(scene, .1);
    if (body_get_centroid(bod1).y > y_max) {
      y_max = body_get_centroid(bod1).y;
    }
  }
  predicted_A = y_max;
  create_drag(scene, 5, bod1);
  for (size_t j = 0; j < 1000; j++) {
    scene_tick(scene, .1);
    assert(body_get_centroid(bod1).y < predicted_A);
  }
  scene_free(scene);
}

void drag_fall_time() {
  body_t *bod1 =
      body_init(new_shape(5, (vector_t){-50, 0}, 2), 5, (rgb_color_t){1, 1, 1});
  body_t *bod2 =
      body_init(new_shape(5, (vector_t){50, 0}, 2), 5, (rgb_color_t){1, 1, 1});
  scene_t *scene = scene_init();
  scene_add_body(scene, bod1);
  scene_add_body(scene, bod2);
  create_newtonian_gravity(scene, 5, bod1, bod2);
  double dt = .01;
  double total_time = 0;
  while (sqrt(
      vec_dot(vec_subtract(body_get_centroid(bod2), body_get_centroid(bod1)),
              vec_subtract(body_get_centroid(bod2), body_get_centroid(bod1))) >
      10)) {
    total_time += dt;
    scene_tick(scene, dt);
  }
  scene_free(scene);
  body_t *bod3 =
      body_init(new_shape(5, (vector_t){-50, 0}, 2), 5, (rgb_color_t){1, 1, 1});
  body_t *bod4 =
      body_init(new_shape(5, (vector_t){50, 0}, 2), 5, (rgb_color_t){1, 1, 1});
  scene_t *scene2 = scene_init();
  scene_add_body(scene2, bod3);
  scene_add_body(scene2, bod4);
  create_newtonian_gravity(scene2, 5, bod3, bod4);
  create_drag(scene2, 2, bod3);
  create_drag(scene2, 2, bod4);
  double time_2 = 0;
  while (sqrt(
      vec_dot(vec_subtract(body_get_centroid(bod4), body_get_centroid(bod3)),
              vec_subtract(body_get_centroid(bod4), body_get_centroid(bod3))) >
      10)) {
    time_2 += dt;
    scene_tick(scene2, dt);
  }
  assert(total_time < time_2);
  scene_free(scene2);
}

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(spring_conservation);
  DO_TEST(drag_spring_dampen_test);
  DO_TEST(drag_fall_time);

  puts("student_tests PASS");
}
