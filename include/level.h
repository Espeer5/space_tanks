#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "body.h"
#include "list.h"
#include "scene.h"
#include "utils.h"

/**
 * A generalization of scene to .
 * The level automatically resizes to store
 * arbitrarily many dynamic and force creators, but has a static
 * number of background objects, and a finite list of 
 * walls.
 */
typedef struct level level_t;

typedef struct trajectory {
    body_t *body;
    list_t *positions;
} trajectory_t;

/**
 * Frees a trajectory pointer and the contained list (but not the body)
 * @param traj pointer to trajectory object to free
 */
void free_traj(trajectory_t *traj);

scene_t *level_scene(level_t *level);

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
 * Predicts the positions of all dynamic bodies in the level, along
 * with extra bodies that can be specified.
 * @param level the level to simulate in
 * @param extras additional bodies not in the scene to simulate
 * @param nsteps the number of steps ahead to simulate
 * @param dt time step between each simulated step
 * @return a list_t* containing trajectory_t objects for each dynamic body
 */
list_t *level_predict(level_t *level, list_t *extras,
                        size_t nsteps, double dt);

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
