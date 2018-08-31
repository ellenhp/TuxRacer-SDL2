/*
 *  bonus.c
 *  tuxracer
 *
 *  Created by emmanuel de Roux on 06/02/09.
 *  Copyright 2009 école Centrale de Lyon. All rights reserved.
 *
 */

#include "bonus.h"
#include "course_load.h"
#include "fonts.h"
#include "gl_util.h"
#include "multiplayer.h"
#include "tuxracer.h"
#include <assert.h>
#include <math.h>

#define BONUS_TIME_ANIMATION 4.0
#define MINIMUM_TRICK_VALUE 10
#define BONUS_DECREMENT_STEPS                                                  \
  20 /* number of time the trick finally worth MINIMUM_TRICK_VALUE */

typedef struct bonus_data {
  int bonus;
  char *bonus_string;
  point_t pos;
  scalar_t start_time;
  struct bonus_data *previous;
  struct bonus_data *next;
} bonus_data_t;

static bonus_data_t *bonus_list = NULL;

static int tricks_count[8];
static int score_for_trick[8];

void bonus_init() {
  int i, f;
  player_data_t *plyr = get_player_data(local_player());

  assert(bonus_list == NULL);

  plyr->bonus_tricks = 0;

  for (i = 0; i < 8; i++) {
    tricks_count[i] = -1;
  }

  /* On assigne des score plus importants aux tricks pour le half pipe, f fois
   * plus important */
  if (!strcmp(get_calculation_mode(), "Half_Pipe")) {
    f = 4;
  } else {
    f = 1;
  }
  score_for_trick[HYPER_HEAVY_JUMP] = 240 * f;
  score_for_trick[RAY_STAR_HYBRID_JUMP] = 250 * f;
  score_for_trick[ROLL_LEFT] = 120 * f;
  score_for_trick[ROLL_RIGHT] = 120 * f;
  score_for_trick[SATURN_ICE_FEVER] = 230 * f;
  score_for_trick[WILD_PINGUIN_SHOW] = 200 * f;
  score_for_trick[BACK_FLIP] = 80 * f;
  score_for_trick[BARLOWS_WHEEL] = 160 * f;
}

static bonus_data_t *create_bonus(const char *text, int value) {
  char bigbuf[256];
  player_data_t *plyr;
  bonus_data_t *ret = calloc(1, sizeof(bonus_data_t));

  ret->bonus = value;

  /*On initialise le nouveau bonus */
  if (value <= 0 || sprintf(bigbuf, "%s: %d pts", text, value) == -1) {
    strncpy(bigbuf, text, sizeof(bigbuf) - 1);
  }

  ret->bonus_string = strdup(bigbuf);

  ret->start_time = g_game.time;

  plyr = get_player_data(local_player());
  ret->pos = plyr->pos;
  return ret;
}

static void delete_bonus(bonus_data_t *bonus) {
  free(bonus->bonus_string);
  free(bonus);
  bonus = NULL;
}

void add_new_bonus(const char *text, int value) {
  bonus_data_t *bonus;
  player_data_t *plyr = get_player_data(local_player());

  if (!bonus_list) {
    bonus_list = create_bonus(text, value);
    plyr->bonus_tricks += value;
    return;
  }

  plyr->bonus_tricks += value;
  bonus = bonus_list;
  bonus_list = create_bonus(text, value);
  bonus_list->next = bonus;
  bonus->previous = bonus_list;
}

static void remove_bonus(bonus_data_t *bonus) {
  if (bonus != NULL) {

    /* On relie la chaine qu'on va briser juste après */
    if (bonus->previous != NULL) {
      (bonus->previous)->next = bonus->next;
    }
    if (bonus->next != NULL) {
      (bonus->next)->previous = bonus->previous;
    }

    if (bonus == bonus_list) {
      bonus_list = bonus->next;
    }

    /*On brise la chaine ! */
    delete_bonus(bonus);
  }
}

static INLINE scalar_t sign(scalar_t a) { return a > 0 ? 1. : -1.; }

static INLINE scalar_t absd(scalar_t a) { return a > 0 ? a : -a; }

static INLINE scalar_t factor_from_time_delta(scalar_t d) {
  static const scalar_t kExplodeAt = 0.2;
  scalar_t t = d / BONUS_TIME_ANIMATION;
  scalar_t s = sign(t);
  t = absd(t);
  if (t <= kExplodeAt)
    return (kExplodeAt - t) * 2;
  t = (t - kExplodeAt) / (1 - kExplodeAt);
  return s * (1 - exp(-5 * t) + t) / (2 - exp(-5));
}

static INLINE scalar_t factor_from_ticks_count(int tc) {
  scalar_t t = (scalar_t)tc / (scalar_t)BONUS_DECREMENT_STEPS;

  return (1 - (1 - exp(-5 * t) + t) / (2 - exp(-5)));
}

void remove_all_bonuses() {
  bonus_data_t *bonus_to_delete = bonus_list;
  bonus_data_t *next_bonus_to_delete;
  while (bonus_to_delete != NULL) {
    next_bonus_to_delete = bonus_to_delete->next;
    remove_bonus(bonus_to_delete);
    bonus_to_delete = next_bonus_to_delete;
  }
}

static colour_t color_from_time_delta(scalar_t time_delta) {
  colour_t colour;
  /* initial colour */
  colour_t ic = {0.95, 0.85, 0.01, 1.0};
  /* final colour */
  colour_t fc = {0.88, 0.41, 0.01, 0.2};
  scalar_t f = factor_from_time_delta(time_delta);
  colour.r = ic.r - f * (ic.r - fc.r);
  colour.g = ic.g - f * (ic.g - fc.g);
  colour.b = ic.b - f * (ic.b - fc.b);
  colour.a = ic.a - f * (ic.a - fc.a);
  return colour;
}

static scalar_t position_from_time_delta(scalar_t time_delta) {
  scalar_t t = time_delta / BONUS_TIME_ANIMATION;
  if (t <= 0.1) {
    return (1 - exp(-5 * t / 0.1) + t / 0.1) / (2 - exp(-5)) / 2.;
  }
  if (t > 0.8)
    return 1;
  if (t >= 0.7) {
    t = t - 0.7;
    return (1 - exp(-5 * t / 0.1) + t / 0.1) / (2 - exp(-5)) / 2. + 0.5;
  }
  return 0.5;
}

static scalar_t scale_from_time_delta(scalar_t time_delta) {
  static const scalar_t kExplodeAt = 0.1;

  scalar_t t = time_delta / BONUS_TIME_ANIMATION;
  if (t < kExplodeAt)
    return 1;
  t = (t - kExplodeAt) / (1 - kExplodeAt);
  return 1. + (1 - exp(-5 * t) + t) / (2 - exp(-5));
}

void draw_bonus(player_data_t *plyr) {
  bonus_data_t *bonus = bonus_list;

  int i = 0;
  while (bonus != NULL) {
    bonus_data_t *next_bonus = bonus->next;
    scalar_t anim_time = g_game.time - bonus->start_time;
    i++;
    if (anim_time < BONUS_TIME_ANIMATION) {
      char *string;
      font_t *font;
      scalar_t width, a, d;

      // Use the same font as for FPS
      char *binding = "bonus";

      if (!get_font_binding(binding, &font)) {
        print_warning(IMPORTANT_WARNING, "Couldn't get font for binding %s",
                      binding);
        return;
      }

      string = bonus->bonus_string;

      set_font_color(font, color_from_time_delta(anim_time));
      set_font_size(font, scale_from_time_delta(anim_time) * 12);
      get_font_metrics_scalar(font, string, &width, &a, &d);

      bind_font_texture(font);
      set_gl_options(TEXFONT);

      /*
      glPushMatrix();
      {
          glTranslatef( 800 * (1 - position_from_time_delta(anim_time)) -
      width/2 - 140, 200.0 + i*18.0, 0 );

          draw_string( font, string );
      }
      glPopMatrix();
       */
    } else {
      remove_bonus(bonus);
    }

    bonus = next_bonus;
  }
}

int get_score_for_trick(int trick) {
  tricks_count[trick]++;
  // Jump mode
  if (!strcmp(get_calculation_mode(), "jump") ||
      !strcmp(get_calculation_mode(), "Half_Pipe")) {
    scalar_t f = factor_from_ticks_count(tricks_count[trick]);
    int score;
    printf("tricks_count : %d et factor = %f\n", tricks_count[trick], f);
    score = score_for_trick[trick];
    return (score * f > MINIMUM_TRICK_VALUE ? score * f : MINIMUM_TRICK_VALUE);
  }

  // default mode
  else {
    return 0;
  }
  return 0;
}