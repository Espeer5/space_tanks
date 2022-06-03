#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "body.h"
#include "list.h"
#include "scene.h"
#include "utils.h"
#include "sdl_wrapper.h"
#include "mystring.h"

/**
 * A generalization of scene to .
 * The level automatically resizes to store
 * arbitrarily many dynamic and force creators, but has a static
 * number of background objects, and a finite list of 
 * walls.
 */
typedef struct level level_t;

/**
 * @brief Returns the number of asteroids present in a given level
 * 
 * @param level 
 * @return size_t 
 */
size_t level_rocks(level_t *level);

/**
 * @brief Returns the scene stored in the level
 * 
 * @param level 
 * @return scene_t* 
 */
scene_t *level_scene(level_t *level);

/**
 * @brief Reads in a line from a file and splits it at all spaces into a string array 
 * 
 * @param f 
 * @return strarray* 
 */
strarray *get_split_line_from_file(FILE *f);

/**
 * Allocates memory for a level based on folders in the path.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated, and that the path is valid.
 *
 * @return the new scene
 */
level_t *level_init_from_folder(char *path, double XMAX, double YMAX);

/**
 * Releases memory allocated for a given level
 * and all the bodies and force creators it contains.
 *
 * @param level a pointer to a level returned from level_init_from_folder()
 */
void level_free(level_t *level);

/**
 * @brief Fires the weapon inluded in the level's scene for the given number enemy
 * 
 * @param scene 
 * @param enemy_num 
 * @return body_t* 
 */
body_t *fire_enemy_weapon(scene_t *scene, size_t enemy_num);

/**
 * @brief Animates an enemy ship included in the level to fire a projectile at the user
 * 
 * @param level 
 * @param enemy_num 
 */
void shoot_as_ai(level_t *level, size_t enemy_num);

/**
 * @brief Reinitializes the user ship and all user weapons whwnwver the player loses a life
 * 
 * @param level 
 */
void int_ship(level_t *level);

/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t level_bodies(level_t *level);

/**
 * Adds a dyanmic body to a level.
 *
 * @param level a pointer to a level 
 * @param body a pointer to the body to add to the scene
 */
void level_add_dynamic_body(level_t *level, body_t *body);

/**
 * Executes a tick of a given level over a small time interval.
 * This requires ticking the component scene, along with any additional
 * features required by a level.
 * If any bodies are marked for removal, they should be removed from the scene
 * and freed, along with any force creators acting on them.
 *
 * @param level a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void level_tick(level_t *level, double dt);

#endif // #ifndef __LEVEL_H__
