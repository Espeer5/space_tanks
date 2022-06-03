#ifndef __SCENE_H__
#define __SCENE_H__

#include "body.h"
#include "list.h"
#include "background.h"
#include <string.h>
#include "weapon.h"

/**
 * A collection of bodies and force creators.
 * The scene automatically resizes to store
 * arbitrarily many bodies and force creators.
 */
typedef struct scene scene_t;

/**
 * A function which adds some forces or impulses to bodies,
 * e.g. from collisions, gravity, or spring forces.
 * Takes in an auxiliary value that can store parameters or state.
 */
typedef void (*force_creator_t)(void *aux);

/**
 * @brief A function to be used as the destructive force from a projectile
 * 
 */
typedef void (*proj_forcer_t)(scene_t *scene, body_t *bod1, body_t *bod2);

/**
 * @brief Allocates memory for an empty scene.
 * Allocates exactly specified amount of space to avoid resizing.
 * Asserts that the required memory is successfully allocated.
 * 
 * @param nbodies 
 * @param nforces 
 * @param user_weapons 
 * @param background_objs 
 * @return scene_t* 
 */
scene_t *scene_init_fixed_size(size_t nbodies, size_t nforces, size_t user_weapons, size_t background_objs);

/**
 * @brief Removes the index'th enemy weapon
 * 
 * @param scene scene to remove weapon from
 * @param index index to remove at
 */
void remove_enemy_weapon(scene_t *scene, size_t index);

/**
 * @brief Adds a weapon tot he scene for use by the user
 * 
 * @param scene 
 * @param weapon 
 * @param force 
 */
void add_user_weapon(scene_t *scene, weapon_t *weapon, proj_forcer_t force);

/**
 * @brief Cycles through user weapons. Intended to be tied to a key press
 * 
 * @param scene 
 */
void change_user_weapon(scene_t *scene);

void scene_add_front(scene_t *scene, body_t *bod);

void wipe_weapons(scene_t *scene);

void rearm(scene_t *scene);

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new scene
 */
scene_t *scene_init(void);

/**
 * @brief Fires the weapon currently selected by the user
 * 
 * @param scene 
 */
body_t *fire_user_weapon(scene_t *scene);

/**
 * @brief Returns the number of background graphic objects in the scene
 * 
 * @param scene 
 * @return size_t 
 */
size_t background_objs(scene_t *scene);

proj_forcer_t get_proj_force(scene_t *scene);

/**
 * @brief Returns the background graphic of the given index
 * 
 * @param scene 
 * @param index 
 * @return list_t* 
 */
background_obj_t *scene_get_background(scene_t *scene, size_t index);

/**
 * @brief Adds the polygon and color given to the background of the scene
 * 
 * @param polygon 
 * @param color 
 */
void scene_add_background(scene_t *scene, list_t *polygon, rgb_color_t color);


void add_enemy_weapon(scene_t *scene, weapon_t* weapon, proj_forcer_t force);

/**
 * Releases memory allocated for a given scene
 * and all the bodies and force creators it contains.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(scene_t *scene);

/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t scene_bodies(scene_t *scene);

/**
 * Gets the body at a given index in a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 * @return a pointer to the body at the given index
 */
body_t *scene_get_body(scene_t *scene, size_t index);

/**
 * Adds a body to a scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param body a pointer to the body to add to the scene
 */
void scene_add_body(scene_t *scene, body_t *body);

/**
 * @deprecated Use body_remove() instead
 *
 * Removes and frees the body at a given index from a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 */
void scene_remove_body(scene_t *scene, size_t index);

/**
 * @deprecated Use scene_add_bodies_force_creator() instead
 * so the scene knows which bodies the force creator depends on
 */
void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer);

/**
 * Adds a force creator to a scene,
 * to be invoked every time scene_tick() is called.
 * The auxiliary value is passed to the force creator each time it is called.
 * The force creator is registered with a list of bodies it applies to,
 * so it can be removed when any one of the bodies is removed.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param forcer a force creator function
 * @param aux an auxiliary value to pass to forcer when it is called
 * @param bodies the list of bodies affected by the force creator.
 *   The force creator will be removed if any of these bodies are removed.
 *   This list does not own the bodies, so its freer should be NULL.
 * @param freer if non-NULL, a function to call in order to free aux
 */
void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer);

void scene_tick_forces(scene_t *scene);

/**
 * @brief Generates a set of randomly shaped and sized background stars in a scene
 * 
 * @param scene 
 * @param num_stars 
 * @param XMAX 
 * @param YMAX 
 */
void generate_back_stars(scene_t *scene, size_t num_stars, double XMAX, double YMAX);

void scene_tick_after_forces(scene_t *scene, double dt);
/**
 * Executes a tick of a given scene over a small time interval.
 * This requires executing all the force creators
 * and then ticking each body (see body_tick()).
 * If any bodies are marked for removal, they should be removed from the scene
 * and freed, along with any force creators acting on them.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void scene_tick(scene_t *scene, double dt);

#endif // #ifndef __SCENE_H__
