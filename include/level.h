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

size_t num_enemies(level_t *level);

size_t level_rocks(level_t *level);

scene_t *level_scene(level_t *level);

list_t *level_get_rocks(level_t *level);

strarray *get_split_line_from_file(FILE *f);

mouse_handler_t *get_mouse_handle(level_t *level);

void set_mouse_handle(level_t *level, mouse_handler_t *handler);

key_handler_t *get_key_handler(level_t *level);

void set_key_handler(level_t *level, key_handler_t *handler);

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


body_t *fire_enemy_weapon(scene_t *scene, size_t enemy_num);

void shoot_as_ai(level_t *level, size_t enemy_num);

void int_ship(level_t *level);

/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t level_bodies(level_t *level);

level_t *get_asteroid_info(char *path);

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
