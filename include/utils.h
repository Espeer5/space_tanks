#ifndef __UTILS_H__
#define __UTILS_H__

#include "color.h"
#include "list.h"
#include "vector.h"
#include <stdlib.h>
#include <time.h>

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
 * @brief Generates a random rgb_color_t by generating three random doubles and
 * storing them in a rgb_color_t struct
 *
 * @return rgb_color_t
 */
rgb_color_t rand_color();

vector_t rand_vec(size_t lower, size_t upper);

/**
 * @brief Seeds random using the current time
 *
 */
void seed_random();

#endif