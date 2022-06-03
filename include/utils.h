#ifndef __UTILS_H__
#define __UTILS_H__

#include "color.h"
#include "list.h"
#include "vector.h"
#include <stdlib.h>
#include <time.h>

/**
 * @brief Creates a square in the scene to be used in testing
 * 
 * @return list_t* 
 */
list_t *make_square();

/**
 * @brief Generates a random double between the values of two passed in size_ts
 *
 * @param lower
 * @param upper
 * @return double
 */
double gen_rand(size_t lower, size_t upper);

/**
 * @brief Generates the graphics for single ship
 * 
 * @param origin 
 * @return list_t* 
 */
list_t *ship_init(vector_t origin);

/**
 * @brief Generates a random rgb_color_t by generating three random doubles and
 * storing them in a rgb_color_t struct
 *
 * @return rgb_color_t
 */
rgb_color_t rand_color();

/**
 * @brief Creates a "dimple" or textural feature on an asteroid providing some extra graphics
 * 
 * @param center 
 * @param radius 
 * @return list_t* 
 */
list_t *dimple_init(vector_t center, double radius);

/**
 * @brief Generates a random vector where both x and y componenets are within the specified bounds
 * 
 * @param lower 
 * @param upper 
 * @return vector_t 
 */
vector_t rand_vec(size_t lower, size_t upper);

/**
 * @brief Creates the outline of an asteroid to be used in asteroid graphics
 * 
 * @param center 
 * @param asteroid_radius 
 * @param num_sides 
 * @return list_t* 
 */
list_t *asteroid_outline_init(vector_t center, double asteroid_radius, double num_sides);

/**
 * @brief Seeds random using the current time
 *
 */
void seed_random();

#endif
