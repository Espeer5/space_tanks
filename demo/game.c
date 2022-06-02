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
#include <stdlib.h>

const double XMAX = 2000;
const double YMAX = 1000;
const size_t BACK_STARS = 100;
const double SHIP_VELOCITY1 = 200;

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

void score_show() {

}

double score_check(state_t *state, char *path) {
  char cwd[100];
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("Current working dir: %s\n", cwd);
   }
  //printf("%s", path);
  FILE *f = fopen(path, "r+");
  assert(f != NULL);
  strarray *info = get_split_line_from_file(f);
  double high_score = atoi(info -> data[0]);
  free_strarray(info);
  if(state -> current_score > high_score) {
    char *score = malloc(2 * sizeof(char));
    sprintf(score, "%f\n", state -> current_score);
    printf("%s\n", score);
    fwrite(score, 1, 2, f);
  }
  fclose(f);
  return state -> current_score;
}

void mouse_follow(state_t *state) {
  vector_t origin = body_get_centroid(scene_get_body(level_scene(state -> level), 0));
  body_set_rotation(scene_get_body(level_scene(state -> level), 0), determine_angle(origin, (vector_t) {mouse_x(state), mouse_y(state)}));
}

state_t *emscripten_init() {
  vector_t min = (vector_t){0, 0};
  vector_t max = (vector_t){XMAX, YMAX};
  sdl_init(min, max);
  state_t *state = malloc(sizeof(state_t));
  state -> level = level_init_from_folder("/levels/level2", XMAX, YMAX);
  state -> current_level = (size_t) 1;
  state -> current_score = 0;
  printf("Welcome to Space Force, The Game!\nControls:\n   Click to rotate and shoot\n   Space: Quick fire\n   Arrow Keys: Maneuver Ship\n   Number keys 1-3: Change weapons\n");
  scene_add_body(level_scene(state->level), body_init(make_square(), 1, (rgb_color_t) {1,0,0}));
  generate_back_stars(level_scene(state -> level), BACK_STARS, XMAX, YMAX);
  return state;
}

void emscripten_main(state_t *state) {
  mouse_follow(state);
  sdl_on_click(mouse_handle);
  sdl_on_key(key_handle);
  sdl_render_scene(level_scene(state->level));
  level_tick(state -> level, time_since_last_tick());
  // score_check(state, "/outputs/score.dat");
}

void emscripten_free(state_t *state) {
  free(state);
}