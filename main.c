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

#include <argp.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SFont.h"

#ifdef DEBUG
#include "debug.h"
#endif

#include "args.h"
#include "common.h"
#include "config.h"
#include "dust.h"
#include "file.h"
#include "globals.h"
#include "mt.h"
#include "rocks.h"
#include "score.h"
#include "shape.h"
#include "sound.h"

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

// Other global variables
char topline[1024];
char *initerror = "";



struct shape shipshape;
float shipx = XSIZE/2, shipy = YSIZE/2;	// X position, 0..XSIZE
float shipdx = SCREENDXMIN, shipdy = 0.0;	// Change in X position per tick.
float screendx = SCREENDXMIN, screendy = 0.0;
float xscroll, yscroll;
float back_dist;

// all movement is based on t_frame.
float t_frame;  // length of this frame (in ticks = 1/20th second)
float s_frame;  // length of this frame (seconds)
int ms_frame;   // length of this frame (milliseconds)
int ms_end;     // end of this frame (milliseconds)

float bangx, bangy, bangdx, bangdy;

int nships,score;
int gameover;
int jets = 0;

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

#define TO_TICKS(seconds) ((seconds)*20*opt_gamespeed)

// ************************************* FUNCS

void
init_engine_dots() {
	int i;
	for(i = 0; i<MAXENGINEDOTS; i++) {
		edot[i].active = 0;
	}
}

void
new_bang_dots(int xbang, int ybang, int dx, int dy, SDL_Surface *s, int power)
{
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

					theta = frnd()*M_PI*2;

					r = 1-(frnd()*frnd());

					bdot[bd2].dx = (power/50.0)*45.0*cos(theta)*r + dx;
					bdot[bd2].dy = (power/50.0)*45.0*sin(theta)*r + dy;
					bdot[bd2].x = x + xbang;
					bdot[bd2].y = y + ybang;

					// Replace the last few bang dots with the pixels from the exploding object
					bdot[bd2].c = (endcount>0)?c:0;
					bdot[bd2].life = 100;
					bdot[bd2].decay = frnd()*3 + 1;
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
			bdot[i].x += bdot[i].dx*t_frame - xscroll;
			bdot[i].y += bdot[i].dy*t_frame - yscroll;

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
draw_engine_dots(SDL_Surface *s) {
	int i;
	Uint16 *rawpixel;
	rawpixel = (Uint16 *) s->pixels;

	for(i = 0; i<MAXENGINEDOTS; i++) {
		if(edot[i].active) {
			edot[i].x += edot[i].dx*t_frame - xscroll;
			edot[i].y += edot[i].dy*t_frame - yscroll;
			if((edot[i].life -= t_frame*3)<0 || edot[i].y<0 || edot[i].y>YSIZE) {
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
new_engine_dots(int n, int dir) {
	int i;
	float a, r;  // angle, random length
	float dx, dy;
	float hx, hy; // half ship width/height.
	static const int s[4] = { 2, 1, 0, 1 };

	hx = surf_ship->w / 2;
	hy = surf_ship->h / 2;

	for(i = 0; i<n; i++) {
		if(dotptr->active == 0) {
			a = frnd()*M_PI + (dir-1)*M_PI_2;
			r = sin(frnd()*M_PI);
			dx = r * cos(a);
			dy = r * -sin(a);  // screen y is "backwards".

			dotptr->active = 1;
			dotptr->x = shipx + s[dir]*hx + (frnd()-0.5)*3;
			dotptr->y = shipy + s[(dir+1)&3]*hy + (frnd()-0.5)*3;
			if(dir&1) {
				dotptr->dx = shipdx + 2*dx;
				dotptr->dy = shipdy + 20*dy;
				dotptr->life = 60 * fabs(dy);
			} else {
				dotptr->dx = shipdx + 20*dx;
				dotptr->dy = shipdy + 2*dy;
				dotptr->life = 60 * fabs(dx);
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

	// Create engine dots out the side we're moving from
	for(m = 0; m<4; m++) {
		if(jets & 1<<m) { // 'jets' is a bit field
			new_engine_dots(80,m);
		}
	}

	move_dust();

	SDL_LockSurface(s);
	draw_dust(s);
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

	play_tune(TUNE_TITLE_PAGE);

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
	init_dust();

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
			SDL_SetAlpha(surf_b_game, SDL_SRCALPHA, (int)(fadegame*(200 + 55*cos(fadetimer += t_frame/1.0))));
			SDL_BlitSurface(surf_b_game,NULL,surf_screen,&dest);

			dest.x = (XSIZE-surf_b_over->w)/2;
			dest.y = (YSIZE-surf_b_over->h)/2 + 40;
			SDL_SetAlpha(surf_b_over, SDL_SRCALPHA, (int)(fadeover*(200 + 55*sin(fadetimer))));
			SDL_BlitSurface(surf_b_over,NULL,surf_screen,&dest);
		break;

		case TITLE_PAGE:

			dest.x = (XSIZE-surf_b_variations->w)/2 + cos(fadetimer/6.5)*10;
			dest.y = (YSIZE/2-surf_b_variations->h)/2 + sin(fadetimer/5.0)*10;
			SDL_SetAlpha(surf_b_variations, SDL_SRCALPHA, (int)(200 + 55*sin(fadetimer += t_frame/2.0)));
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
			play_tune(TUNE_HIGH_SCORE_ENTRY);
			if(!process_score_input()) {  // done inputting name

				// Change state to briefly show high scores page
				state = HIGH_SCORE_DISPLAY;
				state_timeout = 200;

				// Write the high score table to the file
				write_high_score_table();
		
				play_tune(TUNE_TITLE_PAGE);
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

	ms_frame = SDL_GetTicks() - ms_end;
	ms_end += ms_frame;
	if(ms_frame>200 || ms_frame<0) {
		// We won't run at all below 5 frames per second.
		// This also happens if we were paused, grr.
		s_frame = 0;
		ms_frame = 0;
	} else {
		s_frame = opt_gamespeed * ms_frame / 1000;
		if(state == GAMEPLAY) score += ms_frame;
	}
	t_frame = s_frame * 20;

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

			if((state_timeout -= t_frame*3) < 0) {
				switch(state) {
					case DEAD_PAUSE:
						// Create a new ship and start all over again
						state = GAMEPLAY;
						play_tune(TUNE_GAMEPLAY);
						break;
					case GAME_OVER:
						if(new_high_score(score)) {
							SDL_Event e;
							state = HIGH_SCORE_ENTRY;
							state_timeout = 5.0e6;
							SDL_EnableUNICODE(1);
							while(SDL_PollEvent(&e))
								;
						} else if(!keystate[SDLK_SPACE]) {
							state = HIGH_SCORE_DISPLAY;
							state_timeout = 400;
						}
						break;
					case HIGH_SCORE_DISPLAY:
						state = TITLE_PAGE;
						state_timeout = 500.0;
						break;
					case HIGH_SCORE_ENTRY:
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

			// INERTIA
			shipx += shipdx*t_frame;
			shipy += shipdy*t_frame;

			// SCROLLING
			tmp = (shipy-YSCROLLTO)/25 + (shipdy-screendy);
			screendy += tmp * t_frame/12;
			tmp = (shipx-XSCROLLTO)/25 + (shipdx-screendx);
			screendx += tmp * t_frame/12;

			// taper off if we would hit the barrier in under 2 seconds.
			if(back_dist + (screendx - SCREENDXMIN)*TO_TICKS(2) < 0) {
				screendx = SCREENDXMIN - (back_dist/TO_TICKS(2));
			}

			xscroll = screendx * t_frame;
			yscroll = screendy * t_frame;
			back_dist += (screendx - SCREENDXMIN)*t_frame;
			if(opt_max_lead >= 0) back_dist = min(back_dist, opt_max_lead);

			shipx -= xscroll;
			shipy -= yscroll;

			// move bang center
			bangx += bangdx*t_frame - xscroll;
			bangy += bangdy*t_frame - yscroll;

			move_rocks();


			// BOUNCE X
			if(shipx<0 || shipx>XSIZE-surf_ship->w) {
				// BOUNCE from left and right wall
				shipx -= (shipdx-screendx)*t_frame;
				shipdx = screendx - (shipdx-screendx)*opt_bounciness;
			}

			// BOUNCE Y
			if(shipy<0 || shipy>YSIZE-surf_ship->h) {
				// BOUNCE from top and bottom wall
				shipy -= (shipdy-screendy)*t_frame;
				shipdy = screendy - (shipdy-screendy)*opt_bounciness;
			}


			if(draw() && state == GAMEPLAY) {
				// Died
				play_sound(SOUND_BANG); // Play the explosion sound
				bangx = shipx; bangy = shipy; bangdx = shipdx; bangdy = shipdy;
				new_bang_dots(shipx,shipy,shipdx,shipdy,surf_ship,30);
				shipdx *= 0.5; shipdy *= 0.5;
				if(shipdx < SCREENDXMIN) shipdx = SCREENDXMIN;
				if(--nships <= 0) {
					state = GAME_OVER;
					gameover = 1;
					shipdx = 8; shipdy = 0;
					state_timeout = 200.0;
					fadetimer = 0.0;
					faderate = t_frame;
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
				play_tune(TUNE_GAMEPLAY);

				gameover = 0;
				shipx = XSIZE/2.2; shipy = YSIZE/2;
				shipdx = screendx; shipdy = screendy;
			}

			jets = 0;
		} else {
			SDL_PumpEvents();
			keystate = SDL_GetKeyState(NULL);
		}

		if(state == GAMEPLAY) {
			if(!gameover) {

				if(!paused) {
					if(keystate[SDLK_LEFT]  | keystate[SDLK_h]) { shipdx -= 1.5*t_frame; jets |= 1<<0;}
					if(keystate[SDLK_DOWN]  | keystate[SDLK_t]) { shipdy += 1.5*t_frame; jets |= 1<<1;}
					if(keystate[SDLK_RIGHT] | keystate[SDLK_n]) { shipdx += 1.5*t_frame; jets |= 1<<2;}
					if(keystate[SDLK_UP]    | keystate[SDLK_c]) { shipdy -= 1.5*t_frame; jets |= 1<<3;}
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

	gameloop();

	return 0;
}
