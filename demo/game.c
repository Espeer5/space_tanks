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
const double ENEMY_VELO = 100;
const double WALL_GAP = 50;
const double WALL_FORCE = 100;

typedef struct state {
  level_t *level;
  double current_score;
  size_t current_level;
  size_t lives;
} state_t;

size_t get_score(state_t *state) {
  return (size_t)(state -> current_score);
}

void check_score(state_t *state) {
  printf("Current score: %zu\n", get_score(state));
}

void key_handle(char key, key_event_type_t type, double held_time,
                state_t *state) {
  if (type == KEY_PRESSED) {
    body_t *proj;
    switch (key) {
    case 'a':
    body_set_rotation(scene_get_body(level_scene(state -> level), 0), M_PI / 2);
      body_set_velocity(scene_get_body(level_scene(state -> level), 0),
                        (vector_t){-SHIP_VELOCITY1, 0});
      break;
    case 'd':
      body_set_rotation(scene_get_body(level_scene(state -> level), 0),  3 * M_PI / 2);
      body_set_velocity(scene_get_body(level_scene(state -> level), 0),
                        (vector_t){SHIP_VELOCITY1, 0});
      break;
    case 'w':
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
    case 's':
      body_set_rotation(scene_get_body(level_scene(state -> level), 0), M_PI);
      body_set_velocity(scene_get_body(level_scene(state -> level), 0),
                        (vector_t){0, -SHIP_VELOCITY1});
      break;
    case 'c':
      check_score(state);
      break;
    case  'v':
      change_user_weapon(level_scene(state -> level));
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
    velo = vec_multiply(1 / vmag, velo);
    if (vec_dot(velo, gap) > 0) {
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
  size_t counter = 1;
  for (size_t i = 0; i < level_bodies(state->level); i++) {
    scene_t *scene = level_scene(state->level);
    if (!strcmp((char *)body_get_info(scene_get_body(scene, i)),
                    "alien")) {
      dodge(state, scene_get_body(scene, i));
      if (gen_rand(0, 1000) > 985) {
        shoot_as_ai(state->level, counter);
      }
      counter++;
    }
  }
}

double loop_position(double position, double min, double max) {
  // Note: this doesn't account for going way out of bounds by multiple times
  if (position < min) {
    return position + (max - min);
  }
  if (position > max) {
    return position - (max - min);
  }
  return position;
}

state_t *emscripten_init() {
  seed_random();
  vector_t min = (vector_t){0, 0};
  vector_t max = (vector_t){XMAX, YMAX};
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  state -> level = level_init_from_folder("/levels/level1", XMAX, YMAX);
  state -> current_level = 1;
  state -> current_score = 0;
  state -> lives = 3;
  printf("Welcome to Space Force, The Game!\nControls:\n   Click to rotate and shoot\n   Space: Quick fire\n   a/w/s/d: Maneuver Ship\n   v Change weapons\n   c: Check score\n\n Current Score: %zu\n", (size_t) get_score(state));
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

void handle_lose(state_t *state) {
  if(state -> lives > 0) {
    printf("You suck, you just lost a life :(\nLives remaining: %zu\n", state -> lives);
    state -> lives -= 1;
    int_ship(state -> level);
  }
  else {
    printf("Yep, you're trash, game over\nScore: %zu\n", (size_t)state -> current_score);
    exit(0);
  }
}

void advance_level(state_t *state) {
  if(state -> current_level == 1) {
    state -> level = level_init_from_folder("/levels/level2", XMAX, YMAX);
  }
  else if(state -> current_level == 2) {
    state -> level = level_init_from_folder("/levels/level3", XMAX, YMAX);
  }
  else if(state -> current_level == 3) {
    state -> level = level_init_from_folder("/levels/level4", XMAX, YMAX);
  }
  else if(state -> current_level == 4) {
    state -> level = level_init_from_folder("/levels/level5", XMAX, YMAX);
  }
  else if(state -> current_level == 5) {
    state -> level = level_init_from_folder("/levels/level1", XMAX, YMAX);
    state -> current_level = 0;
    state -> lives += 1;
    printf("Extra life granted");
  }
  state -> current_level += 1;
  state -> current_score += 100;
  printf("Congratulations, advance to level %zu\nCurrent Score: %zu\n", state -> current_level, (size_t) get_score(state));
  generate_back_stars(level_scene(state -> level), BACK_STARS, XMAX, YMAX);
}

void emscripten_main(state_t *state) {
  if(strcmp(body_get_info(scene_get_body(level_scene(state -> level), 0)), "ship")) {
    handle_lose(state);
  }
  if(strcmp(body_get_info(scene_get_body(level_scene(state -> level), 1)), "alien")) {
    advance_level(state);
  }
  mouse_follow(state);
  sdl_on_click(mouse_handle);
  sdl_on_key(key_handle);
  sdl_render_scene(level_scene(state->level));
  level_tick(state -> level, time_since_last_tick());
  play_AI(state);
  vector_t ship_position = body_get_centroid(scene_get_body(level_scene(state -> level), 0));
  ship_position.x = loop_position(ship_position.x, 0, XMAX);
  ship_position.y = loop_position(ship_position.y, 0, YMAX);
  body_set_centroid(scene_get_body(level_scene(state -> level), 0), (vector_t) {ship_position.x, ship_position.y});
  body_cleanup(state);
}

void emscripten_free(state_t *state) {
  free(state);
}