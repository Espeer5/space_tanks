#include "utils.h"
#include "list.h"
#include <math.h>
#include "body.h"
#include "constants.h"

const double CIRC_PPOINTS = 65;

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

list_t *asteroid_outline_init(vector_t center, double asteroid_radius, double num_sides) {
  list_t *asteroid = list_init(num_sides, free);
  double position_x = center.x;
  double position_y = center.y;
  double angle = 0;
  double d_angle = (2 * M_PI) / num_sides; 
  for (size_t i = 0; i < num_sides; i++){
    position_x = center.x + (asteroid_radius * cos(angle));
    position_y = center.y + (asteroid_radius * sin(angle));
    vector_t *point = malloc(sizeof(vector_t));
    *point = (vector_t) {position_x, position_y};
    list_add(asteroid, point);
    angle = angle + d_angle;
  }
  return asteroid;
}

list_t *dimple_init(vector_t center, double radius) {
  list_t *c = list_init(CIRC_PPOINTS, free);
  for (size_t i = 0; i < CIRC_PPOINTS; i++) {
    double angle = i * ((2 * M_PI ) / CIRC_PPOINTS);
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t){center.x + (radius * cos(angle)),
                    center.y + (radius * sin(angle))};
    list_add(c, v);
  }
  return c;
}

list_t *ship_init(vector_t origin) {
  size_t steps = shape_steps;
  double dx = SHIP_WIDTH / (float)steps;
  double x = SHIP_WIDTH / 2;
  list_t *ship = list_init(steps * 2, free);
  for (size_t i = 0; i < steps; i++) {
    vector_t *new_vec = malloc(sizeof(vector_t));
    if (-SHIP_COCKPIT_BOUND < x && x < SHIP_COCKPIT_BOUND) {
      *new_vec = (vector_t){
          origin.x + x,
          origin.y + ((-1 * pow(x, 4) / SHIP_COCKPIT_WIDTH_CONSTANT) +
                      SHIP_WIDTH / 2)};
    } else if (-SHIP_WEAPON_BOUND < x && x < -SHIP_WEAPON_OUTSIDE_EDGE) {
      *new_vec = (vector_t){x + origin.x,
                            origin.y + (-SHIP_WEAPON_WIDTH_CONSTANT *
                                            fabs(x + SHIP_WEAPON_OFFSET) +
                                        SHIP_WEAPON_HEIGHT)};
    } else if (SHIP_WEAPON_OUTSIDE_EDGE < x && x < SHIP_WEAPON_BOUND) {
      *new_vec = (vector_t){origin.x + x,
                            origin.y + (-SHIP_WEAPON_WIDTH_CONSTANT *
                                            fabs(x - SHIP_WEAPON_OFFSET) +
                                        SHIP_WEAPON_HEIGHT)};
    } else {
      *new_vec = (vector_t){origin.x + x,
                            origin.y + (-SHIP_WING_WIDTH_CONSTANT * fabs(x) +
                                        SHIP_WEAPON_OFFSET)};
    }
    list_add(ship, new_vec);
    x = x - dx;
  }
  x = -SHIP_WIDTH / 2;
  for (size_t j = 0; j < steps; j++) {
    vector_t *new_vec = malloc(sizeof(vector_t));
    if (-SHIP_WING_SLANT_BOUND < x && x < SHIP_WING_SLANT_BOUND) {
      *new_vec = (vector_t){origin.x + x, origin.y + fabs(x)};
    } else if ((-SHIP_WING_SLANT_OUTER_BOUND < x &&
                -SHIP_WING_SLANT_BOUND > x) ||
               (SHIP_WING_SLANT_BOUND < x && x < SHIP_WING_SLANT_OUTER_BOUND)) {
      *new_vec = (vector_t){
          origin.x + x, origin.y + (-SHIP_AIR_FOIL_WIDTH_CONSTANT * fabs(x) +
                                    SHIP_AIRFOIL_OFFSET)};
    } else {
      *new_vec = (vector_t){origin.x + x, origin.y};
    }
    list_add(ship, new_vec);
    x = x + dx;
  }
  return ship;
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
