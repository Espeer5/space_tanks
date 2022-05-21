#include "weapon.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

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

body_t *gen_projectile(weapon_t *weapon) {
  char *info = malloc(11 * sizeof(char));
  strcpy(info, "projectile");
  body_t *proj = body_init_with_info((weapon -> draw_proj)((vector_t) {body_get_centroid(weapon -> source).x + (7 * (body_get_angle(weapon -> source))), body_get_centroid(weapon -> source).y - (7 * sin(body_get_angle(weapon -> source)))}), weapon -> proj_mass, weapon -> proj_color, info, free);
  body_set_rotation(proj, body_get_angle(weapon -> source));
  body_set_velocity(proj, (vector_t) {(weapon -> proj_velo) * sin(body_get_angle(weapon -> source)), (weapon -> proj_velo) * cos(body_get_angle(weapon -> source))});
  return proj;
}

rgb_color_t get_weapon_color(weapon_t *weapon) { return weapon->proj_color; }

draw_proj_t get_draw_proj(weapon_t *weapon) { return weapon->draw_proj; }

double get_proj_velo(weapon_t *weapon) { return weapon->proj_velo; }
