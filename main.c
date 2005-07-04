/* Variations on RockDodger
 * Space Rocks copyright (C) 2001 Paul Holt <pad@pcholt.com>
 *
 * Project fork 2004, Jason Woofenden and Joshua Grams.
 * (a whole bunch of modifications and project rename)

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

#ifdef DEBUG
#include "debug.h"
#endif

#include "config.h"
#include "file.h"
#include "globals.h"
#include "rocks.h"
#include "score.h"
#include "shape.h"
#include "sound.h"

#include <argp.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SFont.h"

// ************************************* VARS
// SDL_Surface global variables
SDL_Surface 
	*surf_screen,	// Screen
	*surf_b_variations, // "variations" banner
	*surf_b_on, // "on" banner
	*surf_b_rockdodger, // "rockdodger" banner
	*surf_b_game,	// Title element "game"
	*surf_b_over,	// Title element "over"
	*surf_ship,		// Spaceship element
	*surf_life,	// Indicator of number of ships remaining
	*surf_rock[NROCKS],	// THE ROCKS
	*surf_font_big;	// The big font
	

SFont_Font *g_font;

// Structure global variables
struct enginedots edot[MAXENGINEDOTS], *dotptr = edot;
struct bangdots bdot[MAXBANGDOTS], *bdotptr = bdot;
struct spacedot sdot[MAXSPACEDOTS];

// Other global variables
char topline[1024];
char *initerror = "";

// Command-line argument parsing
int opt_fullscreen;
int opt_sound;
int opt_music;
float opt_gamespeed;
int opt_tail_engine;
int opt_friction;

const char *argp_program_version = "Variations on Rockdodger " VERSION;
const char *argp_program_bug_address = "<josh@qualdan.com>";
static char doc[] = "VoR: Dodge the rocks until you die.";
static struct argp_option opts[] = {
	{0, 0, 0, 0, "Basic Options:", 0},
	{"full-screen", 'f', 0, 0, "", 0},
	{"music", 'm', 0, 0, "Enable music", 0},
	{"silent", 's', 0, 0, "Turn off explosion sounds", 0},
	{0, 0, 0, 0, "Gameplay Options:", 1},
	{"game-speed", 'g', "N%", 0, "Game speed [50-100%]", 1},
	{"engine", 'e', 0, 0, "Display large tail plume", 1},
	{"old-physics", 'p', 0, 0, "Original physics (i.e. friction).", 1},
	{0}
};
error_t parse_opt(int, char*, struct argp_state *);
static struct argp argp = { opts, &parse_opt, 0, doc };


struct shape shipshape;
float shipx = XSIZE/2, shipy = YSIZE/2;	// X position, 0..XSIZE
float shipdx = 7.5, shipdy = 0.0;	// Change in X position per tick.
float screendx = 7.5, screendy = 0.0;
float xscroll, yscroll;
float gamerate;  // this controls the speed of everything that moves.

float bangx, bangy, bangdx, bangdy;

int nships,score,ticks_since_last,last_ticks;
int gameover;
int maneuver = 0;

float fadetimer = 0, faderate;

int pausedown = 0, paused = 0;

// bangdot start (bd1) and end (bd2) position:
int bd1 = 0, bd2 = 0;

enum states {
	TITLE_PAGE,
	GAMEPLAY,
	DEAD_PAUSE,
	GAME_OVER,
	HIGH_SCORE_ENTRY,
	HIGH_SCORE_DISPLAY
};
enum states state = TITLE_PAGE;
float state_timeout = 600.0;

#define NSEQUENCE 2
char *sequence[] = {
	"Press SPACE to start",
	"http://herkamire.com/jason/vor"
};

int bangdotlife, nbangdots;
Uint16 heatcolor[W*3];

char *data_dir;
extern char *optarg;
extern int optind, opterr, optopt;

// ************************************* FUNCS

void
init_opts(void)
{
	opt_fullscreen = 0;
	opt_sound = 1;
	opt_music = 0;
	opt_gamespeed = 1.00; // Run game at full speed.
	// These switch back to the old gameplay and are off by default.
	opt_tail_engine = 0;
	opt_friction = 0;
}

error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
	int i;

	switch(key) {
		case 'f': opt_fullscreen = 1; break;
		case 'm': opt_music = 1; break;
		case 's': opt_sound = 0; opt_music = 0; break;
		case 'g': sscanf(arg, "%d%%", &i);
				  if(i < 50) i = 50; else if(i > 100) i = 100;
				  opt_gamespeed = (float)i / 100;
				  break;
		case 'e': opt_tail_engine = 1; break;
		case 'p': opt_friction = 1; break;
		default: break;
	}
	return 0;
}

float
rnd() {
	return (float)random()/(float)RAND_MAX;
}

void
init_engine_dots() {
	int i;
	for(i = 0; i<MAXENGINEDOTS; i++) {
		edot[i].active = 0;
	}
}

void
init_space_dots() {
	int i,b;
	for(i = 0; i<MAXSPACEDOTS; i++) {
		sdot[i].x = rnd()*(XSIZE-5);
		sdot[i].y = rnd()*(YSIZE-5);
		sdot[i].z = MAXDUSTDEPTH*sqrt(rnd());
		b = (MAXDUSTDEPTH - sdot[i].z) * 255.0 / MAXDUSTDEPTH;
		sdot[i].color = SDL_MapRGB(surf_screen->format, b, b, b);
	}
}

void
make_bang_dots(int xbang, int ybang, int dx, int dy, SDL_Surface *s, int power) {

	// TODO - stop generating dots after a certain amount of time has passed, to cope with slower CPUs.
	// TODO - generate and display dots in a circular buffer

	int x,y,endcount;
	Uint16 *rawpixel,c;
	double theta,r;
	int begin_generate;

	begin_generate = SDL_GetTicks();

	SDL_LockSurface(s);
	rawpixel = (Uint16 *) s->pixels;

	//for(n = 0; n <= power/2; n++) {

	endcount = 0;
	while (endcount<3) {
		for(x = 0; x<s->w; x++) {
			for(y = 0; y<s->h; y++) {
				c = rawpixel[s->pitch/2*y + x];
				if(c && c != s->format->colorkey) {

					theta = rnd()*M_PI*2;

					r = 1-(rnd()*rnd());

					bdot[bd2].dx = (power/50.0)*45.0*cos(theta)*r + dx;
					bdot[bd2].dy = (power/50.0)*45.0*sin(theta)*r + dy;
					bdot[bd2].x = x + xbang;
					bdot[bd2].y = y + ybang;

					// Replace the last few bang dots with the pixels from the exploding object
					bdot[bd2].c = (endcount>0)?c:0;
					bdot[bd2].life = 100;
					bdot[bd2].decay = rnd()*3 + 1;
					bdot[bd2].active = 1;

					bd2++;
					bd2 %= MAXBANGDOTS;

					// If the circular buffer is filled, who cares? They've had their chance.
					//if(bd2 == bd1-1) goto exitloop;

				}
			}
		}

		if(SDL_GetTicks() - begin_generate > 7) endcount++;
	}

	SDL_UnlockSurface(s);

}

void
draw_bang_dots(SDL_Surface *s) {
	int i;
	int first_i, last_i;
	Uint16 *rawpixel;
	rawpixel = (Uint16 *) s->pixels;

	first_i = -1;
	last_i = 0;

	for(i = bd1; (bd1 <= bd2)?(i<bd2):(i >= bd1 && i < bd2); last_i = ++i) {

		i %= MAXBANGDOTS;

		if(bdot[i].x <= 0 || bdot[i].x >= XSIZE || bdot[i].y <= 0 || bdot[i].y >= YSIZE) {
			// If the dot has drifted outside the perimeter, kill it
			bdot[i].active = 0;
		}

		if(bdot[i].active) {
			if(first_i < 0)
			first_i = i;
			//last_i = i + 1;
			rawpixel[(int)(s->pitch/2*(int)(bdot[i].y)) + (int)(bdot[i].x)] = bdot[i].c ? bdot[i].c : heatcolor[(int)(bdot[i].life*3)];
			bdot[i].life -= bdot[i].decay;
			bdot[i].x += bdot[i].dx*gamerate - xscroll;
			bdot[i].y += bdot[i].dy*gamerate - yscroll;

			if(bdot[i].life<0)
			bdot[i].active = 0;
		}
	}

	if(first_i >= 0) {
		bd1 = first_i;
		bd2 = last_i;
	}
	else {
		bd1 = 0;
		bd2 = 0;
	}

}


void
draw_space_dots(SDL_Surface *s) {
	int i;
	Uint16 *rawpixel;
	rawpixel = (Uint16 *) s->pixels;

	for(i = 0; i<MAXSPACEDOTS; i++) {
		if(sdot[i].y<0) {
			sdot[i].y = 0;
		}
		rawpixel[(int)(s->pitch/2*(int)sdot[i].y) + (int)(sdot[i].x)] = sdot[i].color;
		sdot[i].x -= xscroll / (1.3 + sdot[i].z);
		sdot[i].y -= yscroll / (1.3 + sdot[i].z);
		if(sdot[i].y >= XSIZE) sdot[i].x -= XSIZE;
		else if(sdot[i].x < 0) sdot[i].x = XSIZE-1;
		if(sdot[i].y > YSIZE) sdot[i].y -= YSIZE;
		else if(sdot[i].y < 0) sdot[i].y += YSIZE-1;
	}
}

void
draw_engine_dots(SDL_Surface *s) {
	int i;
	Uint16 *rawpixel;
	rawpixel = (Uint16 *) s->pixels;

	for(i = 0; i<MAXENGINEDOTS; i++) {
		if(edot[i].active) {
			edot[i].x += edot[i].dx*gamerate - xscroll;
			edot[i].y += edot[i].dy*gamerate - yscroll;
			if((edot[i].life -= gamerate*3)<0 || edot[i].y<0 || edot[i].y>YSIZE) {
				edot[i].active = 0;
			} else if(edot[i].x<0 || edot[i].x>XSIZE) {
				edot[i].active = 0;
			} else {
				int heatindex;
				heatindex = edot[i].life * 6;
				//rawpixel[(int)(s->pitch/2*(int)(edot[i].y)) + (int)(edot[i].x)] = lifecolor[(int)(edot[i].life)];
				rawpixel[(int)(s->pitch/2*(int)(edot[i].y)) + (int)(edot[i].x)] = heatindex>3*W ? heatcolor[3*W-1] : heatcolor[heatindex];
			}
		}
	}
}

void
create_engine_dots(int newdots) {
	int i;
	double theta,r,dx,dy;

	if(!opt_tail_engine) return;

	if(state == GAMEPLAY) {
		for(i = 0; i<newdots*gamerate; i++) {
			if(dotptr->active == 0) {
				theta = rnd()*M_PI*2;
				r = rnd();
				dx = cos(theta)*r;
				dy = sin(theta)*r;

				dotptr->active = 1;
				dotptr->x = shipx + surf_ship->w/2-14;
				dotptr->y = shipy + surf_ship->h/2 + (rnd()-0.5)*5-1;
				dotptr->dx = 10*(dx-1.5) + shipdx;
				dotptr->dy = 1*dy + shipdy;
				dotptr->life = 45 + rnd(1)*5;

				dotptr++;
				if(dotptr-edot >= MAXENGINEDOTS) {
					dotptr = edot;
				}
			}
		}
	}
}

void
create_engine_dots2(int newdots, int m) {
	int i;
	double theta, theta2, dx, dy, adx, ady;

	// Don't create fresh engine dots when
	// the game is not being played and a demo is not beng shown
	if(state != GAMEPLAY) return;

	for(i = 0; i<newdots; i++) {
		if(dotptr->active == 0) {
			theta = rnd()*M_PI*2;
			theta2 = rnd()*M_PI*2;

			dx = cos(theta) * fabs(cos(theta2));
			dy = sin(theta) * fabs(cos(theta2));
			adx = fabs(dx);
			ady = fabs(dy);


			dotptr->active = 1;
			dotptr->x = shipx + surf_ship->w/2 + (rnd()-0.5)*3;
			dotptr->y = shipy + surf_ship->h/2 + (rnd()-0.5)*3;

			switch(m) {
				case 0:
					dotptr->x -= 14;
					dotptr->dx = -20*adx + shipdx;
					dotptr->dy = 2*dy + shipdy;
					dotptr->life = 60 * adx;
				break;
				case 1:
					dotptr->dx = 2*dx + shipdx;
					dotptr->dy = -20*ady + shipdy;
					dotptr->life = 60 * ady;
				break;
				case 2:
					dotptr->x += 14;
					dotptr->dx = 20*adx + shipdx;
					dotptr->dy = 2*dy + shipdy;
					dotptr->life = 60 * adx;
				break;
				case 3:
					dotptr->dx = 2*dx + shipdx;
					dotptr->dy = 20*ady + shipdy;
					dotptr->life = 60 * ady;
				break;
			}
			dotptr++;
			if(dotptr-edot >= MAXENGINEDOTS) {
				dotptr = edot;
			}
		}
	}
}

void
drawdots(SDL_Surface *s) {
	int m;

	// Create more engine dots comin' out da back
	if(!gameover) create_engine_dots(200);

	// Create engine dots out the side we're moving from
	for(m = 0; m<4; m++) {
		if(maneuver & 1<<m) { // 'maneuver' is a bit field
			create_engine_dots2(80,m);
		}
	}

	SDL_LockSurface(s);
	draw_space_dots(s);
	draw_engine_dots(s);
	draw_bang_dots(s);
	SDL_UnlockSurface(s);
}

int
init(void) {

	int i;
	SDL_Surface *temp;
	Uint32 flag;

	// Where are our data files?
	if(!find_files()) exit(1);
	read_high_score_table();

	if(opt_sound) {
		// Initialize SDL with audio and video
		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
			opt_sound = 0;
			printf ("Can't open sound, starting without it\n");
			atexit(SDL_Quit);
		} else {
			atexit(SDL_Quit);
			atexit(SDL_CloseAudio);
			opt_sound = init_sound();
		}
	} else {
		// Initialize with video only
		CONDERROR(SDL_Init(SDL_INIT_VIDEO) != 0);
		atexit(SDL_Quit);
	}

	play_tune(0);

	// Attempt to get the required video size
	flag = SDL_DOUBLEBUF | SDL_HWSURFACE;
	if(opt_fullscreen) flag |= SDL_FULLSCREEN;
	surf_screen = SDL_SetVideoMode(XSIZE,YSIZE,16,flag);

	// Set the title bar text
	SDL_WM_SetCaption("Variations on Rockdodger", "VoR");

	NULLERROR(surf_screen);

	// Set the heat color from the range 0 (cold) to 300 (blue-white)
	for(i = 0; i<W*3; i++) {
		heatcolor[i] = SDL_MapRGB(
			surf_screen->format,
			(i<W)?(i*M/W):(M),(i<W)?0:(i<2*W)?((i-W)*M/W):M,(i<2*W)?0:((i-W)*M/W) // Got that?
		);
	}

	// Load the banners
	NULLERROR(temp = IMG_Load(add_path("banners/variations.png")));
	NULLERROR(surf_b_variations = SDL_DisplayFormat(temp));

	NULLERROR(temp = IMG_Load(add_path("banners/on.png")));
	NULLERROR(surf_b_on = SDL_DisplayFormat(temp));

	NULLERROR(temp = IMG_Load(add_path("banners/rockdodger.png")));
	NULLERROR(surf_b_rockdodger = SDL_DisplayFormat(temp));

	NULLERROR(temp = IMG_Load(add_path("banners/game.png")));
	NULLERROR(surf_b_game = SDL_DisplayFormat(temp));

	NULLERROR(temp = IMG_Load(add_path("banners/over.png")));
	NULLERROR(surf_b_over = SDL_DisplayFormat(temp));

	surf_font_big = IMG_Load(add_path(BIG_FONT_FILE));
	g_font = SFont_InitFont(surf_font_big);

	// Load the spaceship graphic.
	NULLERROR(temp = IMG_Load(add_path("sprites/ship.png")));
	NULLERROR(surf_ship = SDL_DisplayFormat(temp));
	get_shape(surf_ship, &shipshape);

	// Load the life indicator (small ship) graphic.
	NULLERROR(temp = IMG_Load(add_path("indicators/life.png")));
	NULLERROR(surf_life = SDL_DisplayFormat(temp));

	init_engine_dots();
	init_space_dots();

	init_rocks();

	// Remove the mouse cursor
#ifdef SDL_DISABLE
	SDL_ShowCursor(SDL_DISABLE);
#endif

	return 0;
}

int
draw() {
	int i;
	SDL_Rect dest;
	int bang, x;
	char *text;
	float fadegame,fadeover;

	bang = 0;

	// Draw a fully black background
	SDL_FillRect(surf_screen,NULL,0);

	// Draw the background dots
	drawdots(surf_screen);

	// Draw ship
	if(!gameover && state == GAMEPLAY ) {
		dest.x = shipx;
		dest.y = shipy;
		SDL_BlitSurface(surf_ship,NULL,surf_screen,&dest);
	}

	draw_rocks();

	// Draw the life indicators.
	if(state == GAMEPLAY || state == DEAD_PAUSE || state == GAME_OVER)
	for(i = 0; i<nships-1; i++) {
		dest.x = (i + 1)*(surf_life->w + 10);
		dest.y = 20;
		SDL_BlitSurface(surf_life, NULL, surf_screen, &dest);
	}

	// Draw the score
	snprintscore_line(topline, 50, score);
	SFont_Write(surf_screen, g_font, XSIZE-250, 0, topline);

	// If it's game over, show the game over graphic in the dead centre
	switch (state) {
		case GAME_OVER:
			if(fadetimer<3.0/faderate) {
				fadegame = fadetimer/(3.0/faderate);
			} else {
				fadegame = 1.0;
			}

			if(fadetimer<3.0/faderate) {
				fadeover = 0.0;
			} else if(fadetimer<6.0/faderate) {
				fadeover = ((3.0/faderate)-fadetimer)/(6.0/faderate);
			} else {
				fadeover = 1.0;
			}

			dest.x = (XSIZE-surf_b_game->w)/2;
			dest.y = (YSIZE-surf_b_game->h)/2-40;
			SDL_SetAlpha(surf_b_game, SDL_SRCALPHA, (int)(fadegame*(200 + 55*cos(fadetimer += gamerate/1.0))));
			SDL_BlitSurface(surf_b_game,NULL,surf_screen,&dest);

			dest.x = (XSIZE-surf_b_over->w)/2;
			dest.y = (YSIZE-surf_b_over->h)/2 + 40;
			SDL_SetAlpha(surf_b_over, SDL_SRCALPHA, (int)(fadeover*(200 + 55*sin(fadetimer))));
			SDL_BlitSurface(surf_b_over,NULL,surf_screen,&dest);
		break;

		case TITLE_PAGE:

			dest.x = (XSIZE-surf_b_variations->w)/2 + cos(fadetimer/6.5)*10;
			dest.y = (YSIZE/2-surf_b_variations->h)/2 + sin(fadetimer/5.0)*10;
			SDL_SetAlpha(surf_b_variations, SDL_SRCALPHA, (int)(200 + 55*sin(fadetimer += gamerate/2.0)));
			SDL_BlitSurface(surf_b_variations,NULL,surf_screen,&dest);

			dest.x = (XSIZE-surf_b_on->w)/2 + cos((fadetimer + 1.0)/6.5)*10;
			dest.y = (YSIZE/2-surf_b_on->h)/2 + surf_b_variations->h + 20 + sin((fadetimer + 1.0)/5.0)*10;
			SDL_SetAlpha(surf_b_on, SDL_SRCALPHA, (int)(200 + 55*sin(fadetimer-1.0)));
			SDL_BlitSurface(surf_b_on,NULL,surf_screen,&dest);

			dest.x = (XSIZE-surf_b_rockdodger->w)/2 + cos((fadetimer + 2.0)/6.5)*10;
			dest.y = (YSIZE/2-surf_b_rockdodger->h)/2 + surf_b_variations->h + surf_b_on->h + 40 + sin((fadetimer + 2.0)/5)*10;
			SDL_SetAlpha(surf_b_rockdodger, SDL_SRCALPHA, (int)(200 + 55*sin(fadetimer-2.0)));
			SDL_BlitSurface(surf_b_rockdodger,NULL,surf_screen,&dest);

			text = "Version " VERSION;
			x = (XSIZE-SFont_TextWidth(g_font,text))/2 + sin(fadetimer/4.5)*10;
			SFont_Write(surf_screen,g_font,x,YSIZE-50 + sin(fadetimer/2)*5,text);

			text = sequence[(int)(fadetimer/40)%NSEQUENCE];
			//text = "Press SPACE to start!";
			x = (XSIZE-SFont_TextWidth(g_font,text))/2 + cos(fadetimer/4.5)*10;
			SFont_Write(surf_screen,g_font,x,YSIZE-100 + cos(fadetimer/3)*5,text);
		break;

		case HIGH_SCORE_ENTRY:
			play_tune(2);
			if(!process_score_input()) {  // done inputting name

				// Change state to briefly show high scores page
				state = HIGH_SCORE_DISPLAY;
				state_timeout = 200;

				// Write the high score table to the file
				write_high_score_table();
		
				// Play the title page tune
				play_tune(0);
			}
		// FALL THROUGH TO
		case HIGH_SCORE_DISPLAY:
			// Display de list o high scores mon.
			display_scores(surf_screen, 150,50);
			break;
		case GAMEPLAY:
		case DEAD_PAUSE:
			; // no action necessary
	}

	if(!gameover && state == GAMEPLAY) {
		bang = hit_rocks(shipx, shipy, &shipshape);
	}

	ticks_since_last = SDL_GetTicks()-last_ticks;
	last_ticks = SDL_GetTicks();
	if(ticks_since_last>200 || ticks_since_last<0) {
		gamerate = 0;
	}
	else {
		gamerate = opt_gamespeed*ticks_since_last/50.0;
		if(state == GAMEPLAY) {
			score += ticks_since_last;
		}
	}

	// Update the surface
	SDL_Flip(surf_screen);


	return bang;
}

int
gameloop() {
	Uint8 *keystate;
	float tmp;


	for(;;) {
		if(!paused) {
			// Count down the game loop timer, and change state when it gets to zero or less;

			if((state_timeout -= gamerate*3) < 0) {
				switch(state) {
					case DEAD_PAUSE:
						// Create a new ship and start all over again
						state = GAMEPLAY;
						play_tune(1);
						break;
					case GAME_OVER:
						state = HIGH_SCORE_ENTRY;
						state_timeout = 5.0e6;
						if(new_high_score(score)) {
							SDL_Event e;
							SDL_EnableUNICODE(1);
							while(SDL_PollEvent(&e))
								;
						} else {
							state = HIGH_SCORE_DISPLAY;
							state_timeout = 400;
						}
						break;
					case HIGH_SCORE_DISPLAY:
						state = TITLE_PAGE;
						state_timeout = 500.0;
						break;
					case HIGH_SCORE_ENTRY:
						// state = TITLE_PAGE;
						// play_tune(1);
						// state_timeout = 100.0;
						break;
					case TITLE_PAGE:
						state = HIGH_SCORE_DISPLAY;
						state_timeout = 200.0;
						break;
					case GAMEPLAY:
						; // no action necessary
				}
			} else {
				if(state == DEAD_PAUSE) {
					float blast_radius;
					int fixonly;

					if(state_timeout < DEAD_PAUSE_LENGTH - 20.0) {
						blast_radius = BLAST_RADIUS * 1.3;
						fixonly = 1;
					} else {
						blast_radius = BLAST_RADIUS * (DEAD_PAUSE_LENGTH - state_timeout) / 20.0;
						fixonly = 0;
					}
					blast_rocks(bangx, bangy, blast_radius, fixonly);

					if(bangx < 60) bangx = 60;
				}
			}

			new_rocks();

			// FRICTION?
			if(opt_friction) {
				shipdx *= pow((double)0.9,(double)gamerate);
				shipdy *= pow((double)0.9,(double)gamerate);
			}

			// INERTIA
			shipx += shipdx*gamerate;
			shipy += shipdy*gamerate;

			// SCROLLING
			tmp = shipy - (YSIZE / 2);
			tmp += shipdy * 25;
			tmp /= -25;
			tmp = ((screendy * (gamerate - 12)) + (tmp * gamerate)) / 12;
			screendy = -tmp;
			tmp = shipx - (XSIZE / 3);
			tmp += shipdx * 25;
			tmp /= -25;
			tmp = ((screendx * (gamerate - 12)) + (tmp * gamerate)) / 12;
			screendx = -tmp;
			if(screendx < 7.5) screendx=7.5;

			xscroll = screendx * gamerate;
			yscroll = screendy * gamerate;
			shipx -= xscroll;
			shipy -= yscroll;

			// move bang center
			bangx += bangdx*gamerate - xscroll;
			bangy += bangdy*gamerate - yscroll;

			move_rocks();


			// BOUNCE X
			if(shipx<0 || shipx>XSIZE-surf_ship->w) {
				// BOUNCE from left and right wall
				shipx -= (shipdx-screendx)*gamerate;
				shipdx = 2*screendx-shipdx;
			}

			// BOUNCE Y
			if(shipy<0 || shipy>YSIZE-surf_ship->h) {
				// BOUNCE from top and bottom wall
				shipy -= (shipdy-screendy)*gamerate;
				shipdy = 2*screendy-shipdy;
			}


			if(draw() && state == GAMEPLAY) {
				// Died
				play_sound(0); // Play the explosion sound
				bangx = shipx; bangy = shipy; bangdx = shipdx; bangdy = shipdy;
				make_bang_dots(shipx,shipy,shipdx,shipdy,surf_ship,30);
				shipdx *= 0.5; shipdy *= 0.5;
				if(--nships <= 0) {
					state = GAME_OVER;
					gameover = 1;
					shipdx = 8; shipdy = 0;
					state_timeout = 200.0;
					fadetimer = 0.0;
					faderate = gamerate;
				}
				else {
					state = DEAD_PAUSE;
					state_timeout = DEAD_PAUSE_LENGTH;
				}
			}

			SDL_PumpEvents();
			keystate = SDL_GetKeyState(NULL);

			// new game
			if(keystate[SDLK_SPACE] && (state == HIGH_SCORE_DISPLAY || state == TITLE_PAGE)) {

				reset_rocks();

				nships = 4;
				score = 0;

				state = GAMEPLAY;
				play_tune(1);

				gameover = 0;
				shipx = XSIZE/2.2; shipy = YSIZE/2;
				shipdx = screendx; shipdy = screendy;
			}

			maneuver = 0;
		} else {
			SDL_PumpEvents();
			keystate = SDL_GetKeyState(NULL);
		}

		if(state == GAMEPLAY) {
			if(!gameover) {

				if(!paused) {
					if(keystate[SDLK_UP] | keystate[SDLK_c])		{ shipdy -= 1.5*gamerate; maneuver |= 1<<3;}
					if(keystate[SDLK_DOWN] | keystate[SDLK_t])		{ shipdy += 1.5*gamerate; maneuver |= 1<<1;}
					if(keystate[SDLK_LEFT] | keystate[SDLK_h])		{ shipdx -= 1.5*gamerate; maneuver |= 1<<2;}
					if(keystate[SDLK_RIGHT] | keystate[SDLK_n])		{ shipdx += 1.5*gamerate; maneuver |= 1;}
					if(keystate[SDLK_3])		{ SDL_SaveBMP(surf_screen, "snapshot.bmp"); }
				}

				if(keystate[SDLK_p] | keystate[SDLK_s]) {
					if(!pausedown) {
						paused = !paused;
						pausedown = 1;
					}
				} else {
					pausedown = 0;
				}

			}
			else {
				paused = 0;
				pausedown = 0;
			}
		} else if(state == GAME_OVER) {
			if(keystate[SDLK_SPACE]) {
				state_timeout = -1;
			}
		}

		if(state != HIGH_SCORE_ENTRY && (keystate[SDLK_q] || keystate[SDLK_ESCAPE])) {
			return 0;
		}

	}
}

int
main(int argc, char **argv) {
	init_opts();

	argp_parse(&argp, argc, argv, 0, 0, 0);

	if(init()) {
		printf ("ta: '%s'\n",initerror);
		return 1;
	}

	reset_rocks();
	gameloop();

	return 0;
}
