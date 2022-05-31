#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include "color.h"
#include <stdlib.h>

const size_t shape_steps;
const double XMAX;
const double YMAX;
const double UFO_MASS;
const rgb_color_t UFO_COLOR;
const size_t UFO_COLUMNS;
const size_t UFO_ROWS;
const double SHIP_MASS;
const rgb_color_t SHIP_COLOR;
const double UFO_VELO;
const double PROJECTILE_MASS;
const rgb_color_t PROJECTILE_COLOR;
const double PROJECTILE_VELOCITY;
const double SHIP_VELOCITY;
const size_t NUM_ENEMIES;
const double PROJECTILE_OFFSET; // How far from a body does its projectile spawn
const double PROJECTILE_WIDTH;
const double UFO_WIDTH;
const double SHIP_WIDTH;
const double UFO_BUBBLE_UPPER_BOUND;
const double UFO_BUBBLE_WIDTH_CONSTANT;
const double UFO_SAUCER_WIDTH_CONSTANT;
const double UFO_LOWER_SAUCER_UPPER_BOUND;
const double UFO_LOWER_SAUCER_HEIGHT;
const double UFO_LOWER_SAUCER_WIDTH_CONSTANT;
const double UFO_LOWER_OFFSET;
const double UFO_UPPER_OFFSET;
const double UFO_WEAPON_WIDTH_CONSTANT;
const double UFO_WEAPON_OFFSET;
const double UFO_DOWN_TRANSLATION;
const double UFO_SPACING;
const double UFO_WEAPON_BOUND;
const double SHIP_COCKPIT_BOUND;
const double SHIP_COCKPIT_WIDTH_CONSTANT;
const double SHIP_WEAPON_BOUND;
const double SHIP_WEAPON_OUTSIDE_EDGE;
const double SHIP_WEAPON_WIDTH_CONSTANT;
const double SHIP_WEAPON_OFFSET;
const double SHIP_WEAPON_HEIGHT;
const double SHIP_WING_WIDTH_CONSTANT;
const double SHIP_WING_SLANT_BOUND;
const double SHIP_WING_SLANT_OUTER_BOUND;
const double SHIP_AIR_FOIL_WIDTH_CONSTANT;
const double SHIP_AIRFOIL_OFFSET;
const double UFO_WIDTH_SPACING;
const double UFO_HEIGHT;
const double ENEMY_FIRE_RATE_RAND_MAX;
const double ENEMY_FIRE_RATE_CONTROL;
const size_t NUM_BACK_STARS;
const size_t POSITION_APPROXIMATION_ORDER;
const double ANG_VAR;

#endif