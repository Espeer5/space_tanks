#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include "color.h";
#include <stdlib.h>

const size_t shape_steps = 200;
const double XMAX = 2000;
const double YMAX = 700;
const double UFO_MASS = 2;
const rgb_color_t UFO_COLOR = {1, 0, 0};
const size_t UFO_COLUMNS = 4;
const size_t UFO_ROWS = 1;
const double SHIP_MASS = 2;
const rgb_color_t SHIP_COLOR = {0, 0, 1};
const double UFO_VELO = 300;
const double PROJECTILE_MASS = 3;
const rgb_color_t PROJECTILE_COLOR = {0, 1, 0};
const double PROJECTILE_VELOCITY = 500;
const double SHIP_VELOCITY = 300;
const size_t NUM_ENEMIES = 24;
const double PROJECTILE_OFFSET =
    9; // How far from a body does its projectile spawn
const double PROJECTILE_WIDTH = 6;
const double UFO_WIDTH = 80;
const double SHIP_WIDTH = 80;
const double UFO_BUBBLE_UPPER_BOUND = 8.801;
const double UFO_BUBBLE_WIDTH_CONSTANT = 3e8;
const double UFO_SAUCER_WIDTH_CONSTANT = 300;
const double UFO_LOWER_SAUCER_UPPER_BOUND = 14.618;
const double UFO_LOWER_SAUCER_HEIGHT = 13.653;
const double UFO_LOWER_SAUCER_WIDTH_CONSTANT = 10e18;
const double UFO_LOWER_OFFSET = 18;
const double UFO_UPPER_OFFSET = 20;
const double UFO_WEAPON_WIDTH_CONSTANT = 3;
const double UFO_WEAPON_OFFSET = 27;
const double UFO_DOWN_TRANSLATION = 200;
const double UFO_SPACING = 100;
const double UFO_WEAPON_BOUND = 3;
const double SHIP_COCKPIT_BOUND = 6.961;
const double SHIP_COCKPIT_WIDTH_CONSTANT = 100;
const double SHIP_WEAPON_BOUND = 21.579;
const double SHIP_WEAPON_OUTSIDE_EDGE = 18.571;
const double SHIP_WEAPON_WIDTH_CONSTANT = 10;
const double SHIP_WEAPON_OFFSET = 20;
const double SHIP_WEAPON_HEIGHT = 25;
const double SHIP_WING_WIDTH_CONSTANT = .5;
const double SHIP_WING_SLANT_BOUND = 5.6;
const double SHIP_WING_SLANT_OUTER_BOUND = 28;
const double SHIP_AIR_FOIL_WIDTH_CONSTANT = .25;
const double SHIP_AIRFOIL_OFFSET = 7;
const double UFO_WIDTH_SPACING = 70;
const double UFO_HEIGHT = 60;
const double ENEMY_FIRE_RATE_RAND_MAX = 1000;
const double ENEMY_FIRE_RATE_CONTROL =
    400; // Lower number to fire less frequently, higher to fire more frequenctly
const size_t NUM_BACK_STARS = 50;
const size_t POSITION_APPROXIMATION_ORDER = 50;
const double ANG_VAR = 0.1;

#endif