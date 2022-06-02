#include "weapon.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

const double PROJ_OFFSET = 25;

typedef struct weapon {
  draw_proj_t draw_proj;
  double proj_velo;
  rgb_color_t proj_color;
  double proj_mass;
  body_t *source;
} weapon_t;

weapon_t *weapon_init(draw_proj_t draw_proj, double proj_velo,
                      rgb_color_t proj_color, double proj_mass, body_t *source) {
  weapon_t *new_weapon = malloc(sizeof(weapon_t));
  new_weapon->draw_proj = draw_proj;
  new_weapon->proj_velo = proj_velo;
  new_weapon->proj_color = proj_color;
  new_weapon -> source = source;
  new_weapon -> proj_mass = proj_mass;
  return new_weapon;
}

void free_weapon(weapon_t *weapon) {
  free(weapon);
}

list_t *proj_box_init(vector_t center, double width, double height) {
    list_t *box = list_init(4, free);
    vector_t point1 = {center.x - width/2, center.y + height/2};
    vector_t point2 = {center.x + width/2, center.y + height/2};
    vector_t point3 = {center.x + width/2, center.y - height/2};
    vector_t point4 = {center.x - width/2, center.y - height/2};
    vector_t *vec1 = malloc(sizeof(vector_t));
    *vec1 = point1;
    vector_t *vec2 = malloc(sizeof(vector_t));
    *vec2 = point2;
    vector_t *vec3 = malloc(sizeof(vector_t));
    *vec3 = point3;
    vector_t *vec4 = malloc(sizeof(vector_t));
    *vec4 = point4;
    list_add(box, vec1);
    list_add(box, vec2);
    list_add(box, vec3);
    list_add(box, vec4);
    return box;
}


body_t *gen_projectile(weapon_t *weapon) {
  char *info = malloc(11 * sizeof(char));
  strcpy(info, "projectile");
  list_t *proj_graph = (weapon -> draw_proj)((vector_t) {body_get_centroid(weapon -> source).x + (PROJ_OFFSET * cos(body_get_angle(weapon -> source) + (M_PI / 2))), body_get_centroid(weapon -> source).y + (PROJ_OFFSET * sin(body_get_angle(weapon -> source) + (M_PI / 2)))});
  body_t *proj = body_init_with_info(proj_box_init((vector_t) {body_get_centroid(weapon -> source).x + (PROJ_OFFSET * cos(body_get_angle(weapon -> source) + (M_PI / 2))), body_get_centroid(weapon -> source).y + (PROJ_OFFSET * sin(body_get_angle(weapon -> source) + (M_PI / 2)))}, 10, 9), weapon -> proj_mass, (rgb_color_t) {0, 0, 0}, info, free);
  body_add_shape(proj, proj_graph, weapon -> proj_color);
  body_set_rotation(proj, body_get_angle(weapon -> source));
  body_set_velocity(proj, (vector_t) {(weapon -> proj_velo) * cos(body_get_angle(weapon -> source) + (M_PI / 2)), (weapon -> proj_velo) * sin(body_get_angle(weapon -> source) + (M_PI / 2))});
  return proj;
}

rgb_color_t get_weapon_color(weapon_t *weapon) { return weapon->proj_color; }

draw_proj_t get_draw_proj(weapon_t *weapon) { return weapon->draw_proj; }

double get_proj_velo(weapon_t *weapon) { return weapon->proj_velo; }
