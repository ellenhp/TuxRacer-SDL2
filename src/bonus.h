/*
 *  bonus.h
 *  Tux Rider World Challenge
 *
 *  Created by emmanuel de Roux on 06/02/09.
 *  Copyright 2009 école Centrale de Lyon. All rights reserved.
 *
 */
#include "tuxracer.h"

#define HYPER_HEAVY_JUMP 0
#define RAY_STAR_HYBRID_JUMP 1
#define ROLL_LEFT 2
#define ROLL_RIGHT 3
#define SATURN_ICE_FEVER 4
#define WILD_PINGUIN_SHOW 4
#define BACK_FLIP 6
#define BARLOWS_WHEEL 7

void bonus_init();
void add_new_bonus(const char* text, int value);
void remove_all_bonuses();
void draw_bonus(player_data_t * plr);
int get_score_for_trick(int trick);
