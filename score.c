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

#include "font.h"

#include "common.h"
#include "config.h"
#include "file.h"
#include "globals.h"
#include "score.h"

// High score table
struct highscore g_scores[2][N_SCORES] = {
	{
		{1*60*1000,"-"},
		{45*1000,"-"},
		{30*1000,"-"},
		{20*1000,"-"},
		{10*1000,"-"},
		{7*1000,"-"},
		{5*1000,"-"},
		{3*1000,"-"}
	},
	{
		{1*60*1000,"-"},
		{45*1000,"-"},
		{30*1000,"-"},
		{20*1000,"-"},
		{10*1000,"-"},
		{7*1000,"-"},
		{5*1000,"-"},
		{3*1000,"-"}
	}
};

static char *titles[2] = { "Normal\n", "Easy\n" };

int g_easy = 0;
int cur_score = -1; // which score we're currently entering.

void
read_high_score_table()
{
	FILE *f;
	int i, j;
	
	f = open_score_file("r");
	if(f) {
		// If the file exists, read from it
		for(j=0; j<2; j++) {
			fscanf(f, titles[j]);
			for(i = 0; i<N_SCORES; i++) {
				fscanf(f, "%d %31[^\n]\n", &g_scores[j][i].score, g_scores[j][i].name);
			}
		}
		fclose(f);
	}
}

void
write_high_score_table()
{
	FILE *f;
	int i, j;
	
	f = open_score_file("w");
	if(f) {
		// If the file exists, write to it
		for(j=0; j<2; j++) {
			fprintf(f, titles[j]);
			for(i = 0; i<N_SCORES; i++) {
				fprintf (f, "%d %.31s\n", g_scores[j][i].score, g_scores[j][i].name);
			}
		}
		fclose(f);
	}
}

int
score_rank(int score)
{
	int i;

	for(i=0; i<N_SCORES; i++) {
		if(score > g_scores[g_easy][i].score) return i;
	}
	return -1;
}

int
new_high_score(int score)
{
	cur_score = -1;  // assume not a new high score
	if(score <= g_scores[g_easy][LOW_SCORE].score) return false;
	read_high_score_table();
	cur_score = score_rank(score);
	return cur_score >= 0;
}

int
insert_score(int score)
{
	int i;

	// Move all lower scores down a notch, losing lowest score forever.
	if(strcmp(g_scores[g_easy][cur_score].name, "-") != 0)
		for(i=LOW_SCORE; i>cur_score; i--)
			g_scores[g_easy][i] = g_scores[g_easy][i-1];

	// initialize new score entry.
	g_scores[g_easy][cur_score].score = score;
	for(i=0; i<32; i++) g_scores[g_easy][cur_score].name[i] = 0;
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

void
show_score(void)
{
	char s[16];
	int r = snprintf(s, 16, "Time: ");
	snprintscore(s+r, 16-r, score);
	font_write(XSIZE-250, 0, s);
}

void
display_scores(uint32_t x, uint32_t y)
{
	char t[1024];
	int i,h = font_height();

	font_write(x+30, y, "High scores");
	y += h;
	if(g_easy) font_write(x+75,y,"(easy)");
	else font_write(x+60,y,"(normal)");
	for(i = 0; i<N_SCORES; i++) {
		y += h;
		snprintf(t, 1024, "#%1d",i+1);
		font_write(x, y, t);
		snprintscore(t, 1024, g_scores[g_easy][i].score);
		font_write(x+50, y, t);
		if(i == cur_score) snprintf(t, 1024, "%s_", g_scores[g_easy][i].name);
		else snprintf(t, 1024, "%s", g_scores[g_easy][i].name);
		font_write(x+180, y, t);
	}
}

int
process_score_input(SDL_keysym *key)
{
	char *name;
	int n;
	
	name = g_scores[g_easy][cur_score].name;
	n = strlen(name);

	if(key->sym == SDLK_BACKSPACE) {
		if(n > 0) name[--n]=0;
	} else {
		if(key->sym == SDLK_RETURN) {
			SDL_EnableUNICODE(0);
			cur_score = -1;
			if(n == 0) {
				name[0] = '-';
			}
			return false;
		} else if(n < 12) {
			if(key->unicode >= 32 && key->unicode <= 126) {
				name[n++] = key->unicode;
			}
		} // else drop it
	}
	return true;
}
