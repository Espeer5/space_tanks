#ifndef __BACKGROUND_H__
#define __BACKGROUND_H__


#include "list.h"
#include "color.h"
#include "polygon.h"
#include "utils.h"

/**
 * @brief A container for background graphics
 * 
 */
typedef struct background_obj {
  list_t *polygon;
  rgb_color_t color;
} background_obj_t;

/**
 * @brief Initializes a background object to be added to a scene
 * 
 * @param polygon 
 * @param color 
 * @return background_obj_t* 
 */
background_obj_t *obj_init(list_t *polygon, rgb_color_t color);


/**
 * @brief Fres a background object
 * 
 */
void background_obj_free(background_obj_t *obj);


#endif