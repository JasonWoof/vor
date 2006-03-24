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
#include <SDL.h>
#include <SDL_image.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SFont.h"

#include "args.h"
#include "common.h"
#include "config.h"
#include "dust.h"
#include "file.h"
#include "globals.h"
#include "mt.h"
#include "rocks.h"
#include "score.h"
#include "sprite.h"
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

struct ship ship = { SHIP, 0, NULL, XSIZE/2, YSIZE/2, SCREENDXMIN, 0.0 };
	  
float screendx = SCREENDXMIN, screendy = 0.0;
float back_dist;

// all movement is based on t_frame.
float t_frame;  // length of this frame (in ticks = 1/20th second)
int ms_frame;   // length of this frame (milliseconds)
int ms_end;     // end of this frame (milliseconds)

int bang = false;
float bangx, bangy, bangdx, bangdy;

int score;

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
	"http://jasonwoof.org/vor"
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
new_bang_dots(int xbang, int ybang, int dx, int dy, SDL_Surface *s)
{
	int x,y,endcount;
	uint16_t *pixel,c;
	uint32_t colorkey;
	int row_inc;
	double theta,r;
	int begin_generate;

	begin_generate = SDL_GetTicks();
	pixel = s->pixels;
	row_inc = s->pitch/sizeof(uint16_t) - s->w;
	colorkey = s->format->colorkey;

	SDL_LockSurface(s);

	endcount = 0;
	while (endcount<3) {
		pixel = s->pixels;
		for(y=0; y<s->h; y++) {
			for(x = 0; x<s->w; x++) {
				c = *pixel++;
				if(c && c != colorkey) {
					theta = frnd()*M_PI*2;
					r = frnd(); r = 1 - r*r;
					// r = 1 - frnd()*frnd();

					bdot[bd2].dx = 45*r*cos(theta) + dx;
					bdot[bd2].dy = 45*r*sin(theta) + dy;
					bdot[bd2].x = x + xbang;
					bdot[bd2].y = y + ybang;
					bdot[bd2].c = 0;
					bdot[bd2].life = 100;
					bdot[bd2].decay = frnd()*3 + 1;
					bdot[bd2].active = 1;

					// Replace the last few bang dots with the pixels from the exploding object
					if(endcount>0) bdot[bd2].c = c;

					bd2 = (bd2+1) % MAXBANGDOTS;
				}
				pixel += row_inc;
			}
		}
		if(SDL_GetTicks() - begin_generate > 7) endcount++;
	}

	SDL_UnlockSurface(s);
}

void
draw_bang_dots(SDL_Surface *s)
{
	int i;
	int first_i, last_i;
	uint16_t *pixels, *pixel, c;
	int row_inc = s->pitch/sizeof(uint16_t);

	pixels = (uint16_t *) s->pixels;
	first_i = -1;
	last_i = 0;

	for(i=0; i<MAXBANGDOTS; i++) {
		if(!bdot[i].active) continue;

		// decrement life and maybe kill
		bdot[i].life -= bdot[i].decay;
		if(bdot[i].life<0) { bdot[i].active = 0; continue; }

		// move and clip
		bdot[i].x += (bdot[i].dx - screendx)*t_frame;
		bdot[i].y += (bdot[i].dy - screendy)*t_frame;
		if(bdot[i].x < 0 || bdot[i].x >= XSIZE || bdot[i].y < 0 || bdot[i].y >= YSIZE) {
			bdot[i].active = 0;
			continue;
		}

		// check collisions
		if(pixel_collides(bdot[i].x, bdot[i].y)) { bdot[i].active = 0; continue; }

		pixel = pixels + row_inc*(int)(bdot[i].y) + (int)(bdot[i].x);
		if(bdot[i].c) c = bdot[i].c; else c = heatcolor[(int)(bdot[i].life)*3];
		*pixel = c;
	}
}


void
new_engine_dots(int n, int dir) {
	int i;
	float a, r;  // angle, random length
	float dx, dy;
	float hx, hy; // half ship width/height.
	static const int s[4] = { 2, 1, 0, 1 };

	hx = ship.image->w / 2;
	hy = ship.image->h / 2;

	for(i = 0; i<n; i++) {
		if(dotptr->active == 0) {
			a = frnd()*M_PI + (dir-1)*M_PI_2;
			r = sin(frnd()*M_PI);
			dx = r * cos(a);
			dy = r * -sin(a);  // screen y is "backwards".

			dotptr->active = 1;
			dotptr->x = ship.x + s[dir]*hx + (frnd()-0.5)*3;
			dotptr->y = ship.y + s[(dir+1)&3]*hy + (frnd()-0.5)*3;
			if(dir&1) {
				dotptr->dx = ship.dx + 2*dx;
				dotptr->dy = ship.dy + 20*dy;
				dotptr->life = 60 * fabs(dy);
			} else {
				dotptr->dx = ship.dx + 20*dx;
				dotptr->dy = ship.dy + 2*dy;
				dotptr->life = 60 * fabs(dx);
			}

			if(dotptr - edot < MAXENGINEDOTS-1) dotptr++;
			else dotptr = edot;
		}
	}
}

void
draw_engine_dots(SDL_Surface *s) {
	int i;
	uint16_t c;
	uint16_t *pixels = (uint16_t *) s->pixels;
	int row_inc = s->pitch/sizeof(uint16_t);
	int heatindex;

	for(i = 0; i<MAXENGINEDOTS; i++) {
		if(!edot[i].active) continue;
		edot[i].x += (edot[i].dx - screendx)*t_frame;
		edot[i].y += (edot[i].dy - screendy)*t_frame;
		edot[i].life -= t_frame*3;
		if(edot[i].life < 0
				|| edot[i].x<0 || edot[i].x >= XSIZE
				|| edot[i].y<0 || edot[i].y >= YSIZE) {
			edot[i].active = 0;
			continue;
		}
		if(pixel_collides(edot[i].x, edot[i].y)) { edot[i].active = 0; continue; }
		heatindex = edot[i].life * 6;
		c = heatindex>3*W ? heatcolor[3*W-1] : heatcolor[heatindex];
		pixels[row_inc*(int)(edot[i].y) + (int)(edot[i].x)] = c;
	}
}

void
drawdots(SDL_Surface *s) {
	int m;

	// Create engine dots out the side we're moving from
	for(m = 0; m<4; m++) {
		if(ship.jets & 1<<m) { // 'jets' is a bit field
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

SDL_Surface *
load_image(char *filename)
{
	SDL_Surface *tmp, *img = NULL;
	char *s = add_data_path(filename);
	if(s) {
		tmp = IMG_Load(s);
		free(s);
		if(tmp) {
			img = SDL_DisplayFormat(tmp);
			SDL_FreeSurface(tmp);
		}
	}
	return img;
}

void
load_ship(void)
{
	load_sprite(SPRITE(&ship), "sprites/ship.png");
}

int
init(void) {

	int i;
	char *s;
	Uint32 flag;

	// Where are our data files?
	if(!find_files()) exit(1);
	read_high_score_table();

	if(opt_sound) {
		// Initialize SDL with audio and video
		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
			opt_sound = 0;
			fputs("Can't open sound, starting without it\n", stderr);
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
	NULLERROR(surf_b_variations = load_image("banners/variations.png"));
	NULLERROR(surf_b_on = load_image("banners/on.png"));
	NULLERROR(surf_b_rockdodger = load_image("banners/rockdodger.png"));

	NULLERROR(surf_b_game = load_image("banners/game.png"));
	NULLERROR(surf_b_over = load_image("banners/over.png"));

	// Load the life indicator (small ship) graphic.
	NULLERROR(surf_life = load_image("indicators/life.png"));

	// Load the font image
	s = add_data_path(BIG_FONT_FILE);
	if(s) {
		NULLERROR(surf_font_big = IMG_Load(s));
		free(s);
		g_font = SFont_InitFont(surf_font_big);
	}

	init_engine_dots();
	init_dust();

	init_sprites();
	add_sprite(SPRITE(&ship));

	// Remove the mouse cursor
#ifdef SDL_DISABLE
	SDL_ShowCursor(SDL_DISABLE);
#endif

	return 0;
}

void
show_lives(void)
{
	int i;
	SDL_Rect dest;

	for(i=0; i<ship.lives-1; i++) {
		dest.x = (i + 1)*(surf_life->w + 10);
		dest.y = 20;
		SDL_BlitSurface(surf_life, NULL, surf_screen, &dest);
	}
}

void
draw(void) {
	SDL_Rect dest;
	int x;
	char *text;
	float fadegame,fadeover;

	SDL_FillRect(surf_screen,NULL,0);  // black background
	drawdots(surf_screen);             // background dots
	draw_sprite(SPRITE(&ship));
	draw_rocks();

	show_lives();
	show_score();

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

	collisions();

	ms_frame = SDL_GetTicks() - ms_end;
	ms_end += ms_frame;
	t_frame = opt_gamespeed * ms_frame / 50;
	if(state == GAMEPLAY) score += ms_frame;

	// Update the surface
	SDL_Flip(surf_screen);
}

static inline void
kill_ship(Sprite *ship)
{
	ship->flags = MOVE|DRAW;
	SDL_SetAlpha(ship->image, SDL_SRCALPHA, 0);
	bang = true;
}

void
do_collision(Sprite *a, Sprite *b)
{
	if(a->type == SHIP) kill_ship(a);
	else if (b->type == SHIP) kill_ship(b);
	else bounce(a, b);
}

void
gameloop() {
	Uint8 *keystate = SDL_GetKeyState(NULL);
	float tmp;


	for(;;) {
		SDL_PumpEvents();
		keystate = SDL_GetKeyState(NULL);

		if(!paused) {
			// Count down the game loop timer, and change state when it gets to zero or less;

			if((state_timeout -= t_frame*3) < 0) {
				switch(state) {
					case DEAD_PAUSE:
						// Restore the ship and continue playing
						ship.flags = DRAW|MOVE|COLLIDE;
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
					float blast_radius, alpha;
					if(state_timeout >= DEAD_PAUSE_LENGTH - 20.0) {
						blast_radius = BLAST_RADIUS * (DEAD_PAUSE_LENGTH - state_timeout) / 20.0;
						blast_rocks(bangx, bangy, blast_radius);
					}

					if(bangx < 60) bangx = 60;

					alpha = 255.0 * (DEAD_PAUSE_LENGTH - state_timeout) / DEAD_PAUSE_LENGTH;
					SDL_SetAlpha(ship.image, SDL_SRCALPHA, (uint8_t)alpha);
				}
			}

			new_rocks();

			// SCROLLING
			tmp = (ship.y+ship.dy*t_frame-YSCROLLTO)/25 + (ship.dy-screendy);
			screendy += tmp * t_frame/12;
			tmp = (ship.x+ship.dx*t_frame-XSCROLLTO)/25 + (ship.dx-screendx);
			screendx += tmp * t_frame/12;
			// taper off so we don't hit the barrier abruptly.
			// (if we would hit in < 2 seconds, adjust to 2 seconds).
			if(back_dist + (screendx - SCREENDXMIN)*TO_TICKS(2) < 0)
				screendx = SCREENDXMIN - (back_dist/TO_TICKS(2));
			back_dist += (screendx - SCREENDXMIN)*t_frame;
			if(opt_max_lead >= 0) back_dist = min(back_dist, opt_max_lead);

			// move bang center
			bangx += (bangdx - screendx)*t_frame;
			bangy += (bangdy - screendy)*t_frame;

			move_sprites();


			// BOUNCE off left or right edge of screen
			if(ship.x < 0 || ship.x+ship.w > XSIZE) {
				ship.x -= (ship.dx-screendx)*t_frame;
				ship.dx = screendx - (ship.dx-screendx)*opt_bounciness;
			}

			// BOUNCE off top or bottom of screen
			if(ship.y < 0 || ship.y+ship.h > YSIZE) {
				ship.y -= (ship.dy-screendy)*t_frame;
				ship.dy = screendy - (ship.dy-screendy)*opt_bounciness;
			}

			draw();

			if(state == GAMEPLAY && bang) {
				// Died
				bang = false;
				play_sound(SOUND_BANG); // Play the explosion sound
				bangx = ship.x; bangy = ship.y; bangdx = ship.dx; bangdy = ship.dy;
				new_bang_dots(ship.x,ship.y,ship.dx,ship.dy,ship.image);

				if(--ship.lives) {
					state = DEAD_PAUSE;
					state_timeout = DEAD_PAUSE_LENGTH;
					ship.dx = (ship.dx < 0) ? -sqrt(-ship.dx) : sqrt(ship.dx);
					ship.dy = (ship.dy < 0) ? -sqrt(-ship.dy) : sqrt(ship.dy);
					if(ship.dx < SCREENDXMIN) ship.dx = SCREENDXMIN;
				} else {
					state = GAME_OVER;
					ship.dx = SCREENDXMIN; ship.dy = 0;
					state_timeout = 200.0;
					fadetimer = 0.0;
					faderate = t_frame;
				}
			}

			// new game
			if(keystate[SDLK_SPACE]
			   && (state == HIGH_SCORE_DISPLAY
			       || state == TITLE_PAGE
			       || state == GAME_OVER)) {
				reset_sprites();
				reset_rocks();
				screendx = SCREENDXMIN; screendy = 0;

				ship.x = XSIZE/2.2; ship.y = YSIZE/2;
				ship.dx = screendx; ship.dy = screendy;
				ship.lives = 4;
				ship.flags = MOVE|DRAW|COLLIDE;
				SDL_SetAlpha(ship.image, SDL_SRCALPHA, SDL_ALPHA_OPAQUE);
				add_sprite(SPRITE(&ship));

				score = 0;

				state = GAMEPLAY;
				play_tune(TUNE_GAMEPLAY);
			}

			ship.jets = 0;
		}

		if(state == GAMEPLAY) {
			if(!paused) {
				if(keystate[SDLK_LEFT]  | keystate[SDLK_h]) { ship.dx -= 1.5*t_frame; ship.jets |= 1<<0;}
				if(keystate[SDLK_DOWN]  | keystate[SDLK_t]) { ship.dy += 1.5*t_frame; ship.jets |= 1<<1;}
				if(keystate[SDLK_RIGHT] | keystate[SDLK_n]) { ship.dx += 1.5*t_frame; ship.jets |= 1<<2;}
				if(keystate[SDLK_UP]    | keystate[SDLK_c]) { ship.dy -= 1.5*t_frame; ship.jets |= 1<<3;}
				if(keystate[SDLK_3])		{ SDL_SaveBMP(surf_screen, "snapshot.bmp"); }
			}

			if(keystate[SDLK_p] | keystate[SDLK_s]) {
				if(!pausedown) {
					paused = !paused;
					pausedown = 1;
					if(!paused) ms_end = SDL_GetTicks();
				}
			} else {
				pausedown = 0;
			}
		}

		if(state != HIGH_SCORE_ENTRY && (keystate[SDLK_q] || keystate[SDLK_ESCAPE]))
			return;

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
