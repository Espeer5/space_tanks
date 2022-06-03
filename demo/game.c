#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include "level.h"
#include "utils.h"
#include "forces.h"
#include "scene.h"
#include <unistd.h>
#include <stdio.h>

const double XMAX = 2000;
const double YMAX = 1000;
const size_t BACK_STARS = 100;
const double SHIP_VELOCITY1 = 500;
const double ENEMY_VELO = 400;
const double WALL_GAP = 50;
const double WALL_FORCE = 100;

typedef struct state {
  level_t *level;
  double current_score;
  size_t current_level;
} state_t;

void key_handle(char key, key_event_type_t type, double held_time,
                state_t *state) {
  if (type == KEY_PRESSED) {
    body_t *proj;
    switch (key) {
    case LEFT_ARROW:
    body_set_rotation(scene_get_body(level_scene(state -> level), 0), M_PI / 2);
      body_set_velocity(scene_get_body(level_scene(state -> level), 0),
                        (vector_t){-SHIP_VELOCITY1, 0});
      break;
    case RIGHT_ARROW:
      body_set_rotation(scene_get_body(level_scene(state -> level), 0),  3 * M_PI / 2);
      body_set_velocity(scene_get_body(level_scene(state -> level), 0),
                        (vector_t){SHIP_VELOCITY1, 0});
      break;
    case UP_ARROW:
      body_set_rotation(scene_get_body(level_scene(state -> level), 0), 0);
      body_set_velocity(scene_get_body(level_scene(state -> level), 0),
                        (vector_t){0, SHIP_VELOCITY1});
      break;
    case SPACE:
      state -> current_score += 1;
      proj = fire_user_weapon(level_scene(state -> level));
      scene_t *scene = level_scene(state -> level);
      for (size_t i = 1; i < scene_bodies(scene); i++) {
        if (!strcmp((char *)body_get_info(scene_get_body(scene, i)),
                    "alien")) {
          (get_proj_force(scene))(scene, proj, scene_get_body(scene, i));
        }
        else create_physics_collision(scene, .7, proj, scene_get_body(scene, i));
      }
      break;
    case DOWN_ARROW:
      body_set_rotation(scene_get_body(level_scene(state -> level), 0), M_PI);
      body_set_velocity(scene_get_body(level_scene(state -> level), 0),
                        (vector_t){0, -SHIP_VELOCITY1});
      break;
    }
  }
  if (type == KEY_RELEASED) {
    body_set_velocity(scene_get_body(level_scene(state -> level), 0), (vector_t){0, 0});
  }
}

double determine_angle(vector_t origin, vector_t click) {
  click.y = YMAX - click.y;
  vector_t diff = vec_subtract(click, origin);
  if((diff.x > 0 && diff.y < 0) || (diff.x < 0 && diff.y > 0)) {
    if(diff.y > 0) {
      return acos(diff.y / norm(diff));
    }
    else {
      return -acos(diff.y / norm(diff));
    }
  }
  else if(diff.x > 0 && diff.y > 0) {
    return acos(-(diff.y / norm(diff))) + M_PI;
  }
  else {
    return acos(diff.y / norm(diff));
  }
}

void mouse_handle(char button, key_event_type_t type, double mouse_x, double mouse_y, double held_time, state_t *state) {
  if (type == KEY_PRESSED) {
    switch(button) {
      vector_t origin;
      case 0:
        origin = body_get_centroid(scene_get_body(level_scene(state -> level), 0));
        body_set_rotation(scene_get_body(level_scene(state -> level), 0), determine_angle(origin, (vector_t) {mouse_x, mouse_y}));
        body_t *proj = fire_user_weapon(level_scene(state -> level));
        scene_t *scene = level_scene(state -> level);
        for (size_t i = 1; i < scene_bodies(scene); i++) {
        if (!strcmp((char *)body_get_info(scene_get_body(scene, i)),
                    "alien")) {
          (get_proj_force(scene))(scene, proj, scene_get_body(scene, i));
        }
        else create_physics_collision(scene, .7, proj, scene_get_body(scene, i));
      }
      break;
      break;
    }
  }
}

double get_score(state_t *state) {
  return (size_t)(state -> current_score);
}


void mouse_follow(state_t *state) {
  vector_t origin = body_get_centroid(scene_get_body(level_scene(state -> level), 0));
  body_set_rotation(scene_get_body(level_scene(state -> level), 0), determine_angle(origin, (vector_t) {mouse_x(state), mouse_y(state)}));
}


vector_t scale_flee(vector_t vec) {
  double norm = sqrt(vec_dot(vec, vec));
  return vec_multiply(pow(norm, -3), vec);
}

vector_t wall_push(vector_t pos) {
  vector_t push = VEC_ZERO;
  if (pos.x > XMAX - WALL_GAP) {
    push = vec_add(push, (vector_t) {-WALL_FORCE, 0});
  }
  if (pos.x <  WALL_GAP) {
    push = vec_add(push, (vector_t) {WALL_FORCE, 0});
  }
  if (pos.y > YMAX - WALL_GAP) {
    push = vec_add(push, (vector_t) {0, -WALL_FORCE});
  }
  if (pos.y < WALL_GAP) {
    push = vec_add(push, (vector_t) {0, WALL_FORCE});
  }
  return push;
}

void dodge(state_t *state, body_t *enemy) {
  vector_t ecent = body_get_centroid(enemy);
  vector_t total_push = wall_push(ecent);
  for (size_t i = 0; i < level_bodies(state->level); i++) {
    body_t *body = scene_get_body(level_scene(state->level), i);
    vector_t gap = vec_subtract(ecent, body_get_centroid(body));
    vector_t velo = body_get_velocity(body);
    double vmag = sqrt(vec_dot(velo, velo));
    // normalize velocity for convenience
    // Maybe need a sense of urgency?
    velo = vec_multiply(1 / vmag, velo);
    if (vec_dot(velo, gap) > 0) { // object approaching
      double gmag = sqrt(vec_dot(gap, gap));
      // Get perpendicular component
      vector_t closest = vec_subtract(gap, vec_multiply(vec_dot(velo, gap), velo));
      total_push = vec_add(total_push, vec_multiply(vmag, scale_flee(closest)));
    }
  }
  double total_push_mag = sqrt(vec_dot(total_push, total_push));
  if (total_push_mag > 0) {
    body_set_velocity(enemy, vec_multiply(ENEMY_VELO / total_push_mag, total_push));
  }
  else body_set_velocity(enemy, VEC_ZERO);
}

void play_AI(state_t *state) {
  for (size_t i = 0; i < level_bodies(state->level); i++) {
    scene_t *scene = level_scene(state->level);
    if (!strcmp((char *)body_get_info(scene_get_body(scene, i)),
                    "alien")) {
      dodge(state, scene_get_body(scene, i));
    }
  }
}

state_t *emscripten_init() {
  vector_t min = (vector_t){0, 0};
  vector_t max = (vector_t){XMAX, YMAX};
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  state -> level = level_init_from_folder("/levels/level3", XMAX, YMAX);
  state -> current_level = (size_t) 1;
  state -> current_score = 0;
  printf("Welcome to Space Force, The Game!\nControls:\n   Click to rotate and shoot\n   Space: Quick fire\n   Arrow Keys: Maneuver Ship\n   Number keys 1-3: Change weapons\n   S: Check score\n\n Current Score: %zu\n", (size_t) get_score(state));
  scene_add_body(level_scene(state->level), body_init(make_square(), 1, (rgb_color_t) {1,0,0}));
  generate_back_stars(level_scene(state -> level), BACK_STARS, XMAX, YMAX);
  return state;
}

void body_cleanup(state_t *state) {
  for (size_t i = 0; i < level_bodies(state->level); i++) {
    body_t *body = scene_get_body(level_scene(state->level), i);
    vector_t cent = body_get_centroid(body);
    if (cent.x > 2 * XMAX || cent.x < -XMAX || cent.y > 2 * YMAX || cent.y < -YMAX) {
      scene_remove_body(level_scene(state->level), i);
    }
  }
}

void emscripten_main(state_t *state) {
  mouse_follow(state);
  sdl_on_click(mouse_handle);
  sdl_on_key(key_handle);
  sdl_render_scene(level_scene(state->level));
  level_tick(state -> level, time_since_last_tick());
  play_AI(state);
  shoot_as_ai(state -> level, 1);
  body_cleanup(state);
}

void emscripten_free(state_t *state) {
  free(state);
}