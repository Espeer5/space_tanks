#ifndef __WEAPON_H__
#define __WEAPON_H__

#include "body.h"
/**
 * @brief A struct to hold all of the characteristics of a weapon assigned to a
 * body
 *
 */
typedef struct weapon weapon_t;

/**
 * @brief A function that draws the projectile associated with the weapon.
 * Doesn't initialize the body of the projectile simply draws the graphics
 *
 */
typedef void *(*draw_proj_t)(vector_t origin);

/**
 * @brief Initializes a weapon to be assigned to a body
 * 
 * @param draw_proj 
 * @param proj_velo 
 * @param proj_color 
 * @param proj_force 
 * @param proj_mass 
 * @param source 
 * @return weapon_t* 
 */
weapon_t *weapon_init(draw_proj_t draw_proj, double proj_velo,
                      rgb_color_t proj_color, double proj_mass, body_t *source);

/**
 * @brief Frees a weapon object
 * 
 * @param weapon 
 */
void free_weapon(weapon_t *weapon);

/**
 * @brief Generates a projectile body to be added to a scene
 * 
 * @param weapon 
 * @return body_t* 
 */
body_t *gen_projectile(weapon_t *weapon);

/**
 * @brief Get the weapon projectile color
 *
 * @param weapon
 * @return rgb_color_t
 */
rgb_color_t get_weapon_color(weapon_t *weapon);

/**
 * @brief Get the projectile drawing function of a weapon
 *
 * @param weapon
 * @return draw_proj_t
 */
draw_proj_t get_draw_proj(weapon_t *weapon);

/**
 * @brief Get the projectile velocity
 *
 * @param weapon
 * @return double
 */
double get_proj_velo(weapon_t *weapon);

#endif
