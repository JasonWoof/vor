/* Variations on RockDodger
 * Copyright (C) 2004 Joshua Grams <josh@qualdan.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef VOR_SCORE_H
#define VOR_SCORE_H

#include <SDL.h>
#include <inttypes.h>
#include <stdio.h>

#define N_SCORES 8
#define LOW_SCORE (N_SCORES-1)

struct highscore {
	int score;
	char name[32];
};

extern struct highscore g_scores[2][N_SCORES];

void read_high_score_table(void);
void write_high_score_table(void);
int snprintscore(char *s, size_t n, int score);
void show_score(void);
void display_scores(uint32_t x, uint32_t y);
int new_high_score(int score);
int insert_score(int score);
int process_score_input(SDL_keysym *key);

#endif // VOR_SCORE_H
