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

#include <SDL.h>
#include <SDL_keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SFont.h"

#include "common.h"
#include "config.h"
#include "file.h"
#include "score.h"

// High score table
struct highscore g_scores[N_SCORES] = {
	{1*60*1000,"-"},
	{45*1000,"-"},
	{30*1000,"-"},
	{20*1000,"-"},
	{10*1000,"-"},
	{7*1000,"-"},
	{5*1000,"-"},
	{3*1000,"-"}
};

extern SFont_Font *g_font;

int cur_score = -1; // which score we're currently entering.

void
read_high_score_table()
{
	FILE *f;
	int i;
	
	f = open_score_file("r");
	if(f) {
		// If the file exists, read from it
		for(i = 0; i<N_SCORES; i++) {
			fscanf(f, "%d %31[^\n]", &g_scores[i].score, g_scores[i].name);
		}
		fclose(f);
	}
}

void
write_high_score_table()
{
	FILE *f;
	int i;
	
	f = open_score_file("w");
	if(f) {
		// If the file exists, write to it
		for(i = 0; i<N_SCORES; i++) {
			fprintf (f, "%d %.31s\n", g_scores[i].score, g_scores[i].name);
		}
		fclose(f);
	}
}

int
score_rank(int score)
{
	int i;

	for(i=0; i<N_SCORES; i++) {
		if(score > g_scores[i].score) return i;
	}
	return -1;
}

int
new_high_score(int score)
{
	int i;

	cur_score = -1;  // assume not a new high score
	if(score <= g_scores[LOW_SCORE].score) return false;
	read_high_score_table();
	cur_score = score_rank(score);
	if(cur_score < 0) return false;

	// Move all lower scores down a notch, losing lowest score forever.
	for(i=LOW_SCORE; i>cur_score; i--) g_scores[i] = g_scores[i-1];

	// initialize new score entry.
	g_scores[cur_score].score = score;
	for(i=0; i<32; i++) g_scores[cur_score].name[i] = 0;
	return true;
}

int
snprintscore(char *s, size_t n, int score)
{
	int min = score/60000;
	int sec = score/1000%60;
	int tenths = score%1000/100;
	if(min) {
		return snprintf(s, n, "%2d:%.2d.%d", min, sec, tenths);
	} else {
		return snprintf(s, n, "%2d.%d", sec, tenths);
	}
}

int
snprintscore_line(char *s, size_t n, int score)
{
	int r = snprintf(s, n, "Time: ");
	return r + snprintscore(s+r, n-r, score);
}

void
display_scores(SDL_Surface *s, uint32_t x, uint32_t y)
{
	char t[1024];
	int i,h = SFont_TextHeight(g_font);

	SFont_Write(s,g_font,x+30,y,"High scores");
	y += h;
	for(i = 0; i<N_SCORES; i++) {
		y += h;
		snprintf(t, 1024, "#%1d",i+1);
		SFont_Write(s, g_font, x, y, t);
		snprintscore(t, 1024, g_scores[i].score);
		SFont_Write(s, g_font, x+50, y, t);
		if(i == cur_score) snprintf(t, 1024, "%s_", g_scores[i].name);
		else snprintf(t, 1024, "%s", g_scores[i].name);
		SFont_Write(s, g_font, x+180, y, t);
	}
}

int
process_score_input(void)
{
	char *name;
	int c,k,n;
	SDL_Event e;
	
	name = g_scores[cur_score].name;
	n = strlen(name);

	while(SDL_PollEvent(&e) && e.type == SDL_KEYDOWN) {
		c = e.key.keysym.unicode;
		k = e.key.keysym.sym;
		if(k == SDLK_BACKSPACE) {
			if(n > 0) name[--n]=0;
		} else {
			if(k == SDLK_RETURN) {
				SDL_EnableUNICODE(0);
				cur_score = -1;
				return false;
			} else name[n++] = c;
		}
	}
	return true;
}
