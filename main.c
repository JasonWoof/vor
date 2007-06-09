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

#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "font.h"

#include "args.h"
#include "common.h"
#include "config.h"
#include "dust.h"
#include "file.h"
#include "float.h"
#include "globals.h"
#include "mt.h"
#include "rocks.h"
#include "score.h"
#include "sprite.h"
#include "sound.h"
#include "autopilot.h"

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
	

font *g_font;

struct dot {
	int active;
	float x, y;
	float dx, dy;
	float mass;   // in DOT_MASS_UNITs
	float decay;  // rate at which to reduce mass.
	int heat;     // heat multiplier (color).
};

struct dot edot[MAXENGINEDOTS], *dotptr = edot;
struct dot bdot[MAXBANGDOTS];

// Other global variables
char topline[1024];
char *initerror = "";

struct ship ship = { SHIP, 0, NULL, XSIZE/2, YSIZE/2, BARRIER_SPEED, 0.0 };
	  
float screendx = BARRIER_SPEED, screendy = 0.0;
float dist_ahead;

// all movement is based on t_frame.
unsigned long frames, start, end;
float t_frame;  // length of this frame (in ticks = 1/20th second)  adjusted for gamespeed
int ms_frame;   // length of this frame (milliseconds)
int ms_end;     // end of this frame (milliseconds)

float gamespeed = 1.00;

int score;

float fadetimer = 0;

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

#define NSEQUENCE 3
char *msgs[2][3] = {
	{
		"Press SPACE for normal game",
		"Press '1' for easy game",
		"http://jasonwoof.org/vor"
	},
	{
		"Press SPACE for easy game",
		"Press '2' for normal game",
		"http://jasonwoof.org/vor"
	}
};

int bangdotlife, nbangdots;
Uint16 heatcolor[W*3];

char *data_dir;
extern char *optarg;
extern int optind, opterr, optopt;

#define TO_TICKS(seconds) ((seconds)*20*gamespeed)

// ************************************* FUNCS

void
init_engine_dots() {
	int i;
	for(i = 0; i<MAXENGINEDOTS; i++) {
		edot[i].active = 0;
	}
}


void
new_engine_dots(void) {
	int dir, i;
	int n = t_frame * ENGINE_DOTS_PER_TIC;
	float a, r;  // angle, random length
	float dx, dy;
	float hx, hy; // half ship width/height.
	static const int s[4] = { 2, 1, 0, 1 };
	float time;
	float accelh, accelv, past_ship_dx, past_ship_dy;

	hx = ship.image->w / 2;
	hy = ship.image->h / 2;

	for(dir=0; dir<4; dir++) {
		if(!(ship.jets & 1<<dir)) continue;

		for(i = 0; i<n; i++) {
			if(dotptr->active == 0) {
				a = frnd()*M_PI + (dir-1)*M_PI_2;
				r = sin(frnd()*M_PI);
				dx = r * cos(a);
				dy = r * -sin(a);  // screen y is "backwards".

				dotptr->decay = 3;
				dotptr->heat = 6;

				// dot was created at a random time during the time span
				time = frnd() * t_frame; // this is how long ago

				// calculate how fast the ship was going when this engine dot was
				// created (as if it had a smooth acceleration). This is used in
				// determining the velocity of the dots, but not their starting
				// location.
				accelh = ((ship.jets >> 2) & 1) - (ship.jets & 1);
				accelh *= THRUSTER_STRENGTH * time;
				past_ship_dx = ship.dx - accelh;
				accelv = ((ship.jets >> 1) & 1) - ((ship.jets >> 3) & 1);
				accelv *= THRUSTER_STRENGTH * time;
				past_ship_dy = ship.dy - accelv;

				// the starting position (not speed) of the dot is calculated as
				// though the ship were traveling at a constant speed for this
				// t_frame.
				dotptr->x = (ship.x - (ship.dx - screendx) * time) + s[dir]*hx;
				dotptr->y = (ship.y - (ship.dy - screendy) * time) + s[(dir+1)&3]*hy;
				if(dir&1) {
					dotptr->dx = past_ship_dx + 2*dx;
					dotptr->dy = past_ship_dy + 20*dy;
					dotptr->mass = 60 * fabs(dy);
				} else {
					dotptr->dx = past_ship_dx + 20*dx;
					dotptr->dy = past_ship_dy + 2*dy;
					dotptr->mass = 60 * fabs(dx);
				}

				// move the dot as though it were created in the past
				dotptr->x += (dotptr->dx - screendx) * time;
				dotptr->y += (dotptr->dy - screendy) * time;

				if(!fclip(dotptr->x, XSIZE) && !fclip(dotptr->y, YSIZE)) {
					dotptr->active = 1;
					if(dotptr - edot < MAXENGINEDOTS-1) {
						dotptr++;
					} else {
						dotptr = edot;
					}
				}
			}
		}
	}
}


void
new_bang_dots(struct sprite *s)
{
	int i, n, x, y;
	uint16_t *pixel, c;
	uint32_t colorkey;
	int row_inc;
	double theta, r;
	SDL_Surface *img = s->image;

	n = 20;
	pixel = img->pixels;
	row_inc = img->pitch/sizeof(uint16_t) - img->w;
	colorkey = img->format->colorkey;

	if(SDL_MUSTLOCK(img)) { SDL_LockSurface(img); }

	for(i=0; i<n; i++) {
		pixel = img->pixels;
		for(y=0; y<img->h; y++) {
			for(x = 0; x<img->w; x++) {
				c = *pixel++;
				if(c && c != colorkey) {
					theta = frnd()*M_PI*2;
					r = frnd(); r = 1 - r*r;

					bdot[bd2].dx = 45*r*cos(theta) + s->dx;
					bdot[bd2].dy = 45*r*sin(theta) + s->dy;
					bdot[bd2].x = x + s->x;
					bdot[bd2].y = y + s->y;
					bdot[bd2].mass = frnd() * 99;
					bdot[bd2].decay = frnd()*1.5 + 0.5;
					bdot[bd2].heat = 3;
					bdot[bd2].active = 1;

					bd2 = (bd2+1) % MAXBANGDOTS;
				}
				pixel += row_inc;
			}
		}
	}

	if(SDL_MUSTLOCK(img)) { SDL_UnlockSurface(img); }
}


void
move_dot(struct dot *d)
{
	Sprite *hit;
	float mass;

	if(d->active) {
		d->x += (d->dx - screendx) * t_frame;
		d->y += (d->dy - screendy) * t_frame;
		d->mass -= t_frame * d->decay;
		if(d->mass < 0 || fclip(d->x, XSIZE) || fclip(d->y, YSIZE))
			d->active = 0; 
		else {
			hit = pixel_collides(d->x, d->y);
			if(hit) if(hit->type != SHIP) {
				d->active = 0;
				mass = sprite_mass(hit);
				hit->dx += DOT_MASS_UNIT * d->mass * (d->dx - hit->dx) / mass;
				hit->dy += DOT_MASS_UNIT * d->mass * (d->dy - hit->dy) / mass;
			}
		}
	}
}

void
move_dots(void)
{
	int i;

	for(i=0; i<MAXBANGDOTS; i++) move_dot(&bdot[i]);
	for(i=0; i<MAXENGINEDOTS; i++) move_dot(&edot[i]);
}


void
draw_dot(struct dot *d)
{
	uint16_t *pixels, *pixel;
	int row_inc;

	if(d->active) {
		pixels = (uint16_t *) surf_screen->pixels;
		row_inc = surf_screen->pitch / sizeof(uint16_t);
		pixel = pixels + (int)d->y * row_inc + (int)d->x;
		*pixel = heatcolor[min(3*W-1, (int)(d->mass * d->heat))];
	}
}

void
draw_dots(void) {
	int i;

	if(SDL_MUSTLOCK(surf_screen)) { SDL_LockSurface(surf_screen); }
	draw_dust();
	for(i=0; i<MAXBANGDOTS; i++) draw_dot(&bdot[i]);
	for(i=0; i<MAXENGINEDOTS; i++) draw_dot(&edot[i]);
	if(SDL_MUSTLOCK(surf_screen)) { SDL_UnlockSurface(surf_screen); }
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
	load_sprite(SPRITE(&ship), "ship.png");
}

void font_cleanup() {
	font_free(g_font);
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
	NULLERROR(surf_b_variations = load_image("b_variations.png"));
	NULLERROR(surf_b_on = load_image("b_on.png"));
	NULLERROR(surf_b_rockdodger = load_image("b_rockdodger.png"));

	NULLERROR(surf_b_game = load_image("b_game.png"));
	NULLERROR(surf_b_over = load_image("b_over.png"));

	// Load the life indicator (small ship) graphic.
	NULLERROR(surf_life = load_image("life.png"));

	// Load the font image
	s = add_data_path("font.png");
	if(s) {
		g_font = font_load(s);
		atexit(&font_cleanup);
	} else {
		fprintf(stderr, "could create path to font\n");
		exit(1);
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
draw_game_over(void)
{
	int x;
	char *text0, *text1;
	SDL_Rect dest;

	fadetimer += t_frame;

	dest.x = (XSIZE-surf_b_game->w)/2;
	dest.y = (YSIZE-surf_b_game->h)/2-40;
	SDL_BlitSurface(surf_b_game,NULL,surf_screen,&dest);

	dest.x = (XSIZE-surf_b_over->w)/2;
	dest.y = (YSIZE-surf_b_over->h)/2 + 40;
	SDL_BlitSurface(surf_b_over,NULL,surf_screen,&dest);

	if(new_high_score(score)) {
		text0 = "New High Score!";
		text1 = "Press SPACE to continue";
	} else {
		text0 = msgs[g_easy][0];
		text1 = msgs[g_easy][1];
	}

	x = (XSIZE-font_width(text0))/2 + cos(fadetimer/9)*10;
	font_write(x,YSIZE-100 + cos(fadetimer/6)*5,text0);

	x = (XSIZE-font_width(text1))/2 + sin(fadetimer/9)*10;
	font_write(x,YSIZE-50 + sin(fadetimer/4)*5,text1);
}

void
draw_title_page(void)
{
	int x;
	char *text;
	SDL_Rect dest;

	fadetimer += t_frame/2.0;

	dest.x = (XSIZE-surf_b_variations->w)/2 + cos(fadetimer/6.5)*10;
	dest.y = (YSIZE/2-surf_b_variations->h)/2 + sin(fadetimer/5.0)*10;
	SDL_BlitSurface(surf_b_variations,NULL,surf_screen,&dest);

	dest.x = (XSIZE-surf_b_on->w)/2 + cos((fadetimer + 1.0)/6.5)*10;
	dest.y = (YSIZE/2-surf_b_on->h)/2 + surf_b_variations->h + 20 + sin((fadetimer + 1.0)/5.0)*10;
	SDL_BlitSurface(surf_b_on,NULL,surf_screen,&dest);

	dest.x = (XSIZE-surf_b_rockdodger->w)/2 + cos((fadetimer + 2.0)/6.5)*10;
	dest.y = (YSIZE/2-surf_b_rockdodger->h)/2 + surf_b_variations->h + surf_b_on->h + 40 + sin((fadetimer + 2.0)/5)*10;
	SDL_BlitSurface(surf_b_rockdodger,NULL,surf_screen,&dest);

	text = msgs[g_easy][(int)(fadetimer/35)%NSEQUENCE];
	x = (XSIZE-font_width(text))/2 + cos(fadetimer/4.5)*10;
	font_write(x,YSIZE-100 + cos(fadetimer/3)*5,text);

	text = "Version " VERSION;
	x = (XSIZE-font_width(text))/2 + sin(fadetimer/4.5)*10;
	font_write(x,YSIZE-50 + sin(fadetimer/2)*5,text);
}

void
draw(void)
{
	SDL_FillRect(surf_screen,NULL,0);  // black background
	draw_dots();            // background dots
	draw_sprite(SPRITE(&ship));
	draw_rocks();

	show_lives();
	show_score();

	switch (state) {
		case GAME_OVER: draw_game_over(); break;

		case TITLE_PAGE: draw_title_page(); break;

		case HIGH_SCORE_ENTRY: play_tune(TUNE_HIGH_SCORE_ENTRY);
			// and fall through to
		case HIGH_SCORE_DISPLAY:
			// Display de list o high scores mon.
			display_scores(150,50);
			break;
		case GAMEPLAY:
		case DEAD_PAUSE:
			; // no action necessary
	}

	// Update the surface
	SDL_Flip(surf_screen);
}

static inline void
kill_ship(struct ship *ship)
{
	play_sound(SOUND_BANG);
	new_bang_dots(SPRITE(ship));
	if(--ship->lives) {
		state = DEAD_PAUSE;
		state_timeout = DEAD_PAUSE_LENGTH;
		// want ship to be invisible, but keep drifting at sqrt(speed)
		// to leave it in the middle of the space from the explosion.
		ship->flags = MOVE;
		ship->dx = (ship->dx < 0) ? -sqrt(-ship->dx) : sqrt(ship->dx);
		ship->dy = (ship->dy < 0) ? -sqrt(-ship->dy) : sqrt(ship->dy);
		if(ship->dx < BARRIER_SPEED) ship->dx = BARRIER_SPEED;
	} else {
		state = GAME_OVER;
		state_timeout = 200.0;
		fadetimer = 0.0;
		ship->flags = 0;
		// scrolling is based on the ship speed, so we need to reset it.
		ship->dx = BARRIER_SPEED; ship->dy = 0;
	}
}

void
do_collision(Sprite *a, Sprite *b)
{
	if(a->type == SHIP) kill_ship((struct ship *)a);
	else if(b->type == SHIP) kill_ship((struct ship *)b);
	else bounce(a, b);
}

void
init_score_entry(void)
{
	SDL_Event e;
	state = HIGH_SCORE_ENTRY;
	state_timeout = 5.0e6;
	SDL_EnableUNICODE(1);
	while(SDL_PollEvent(&e))
		;
	insert_score(score);
}

// Count down the state timer, and change state when it gets to zero or less;
void
update_state(void)
{
	state_timeout -= t_frame*3;
	if(state_timeout > 0) return;

	switch(state) {
		case GAMEPLAY: break;  // no action necessary
		case DEAD_PAUSE:
			// Restore the ship and continue playing
			ship.flags = DRAW|MOVE|COLLIDE;
			state = GAMEPLAY;
			play_tune(TUNE_GAMEPLAY);
			break;
		case GAME_OVER:
			if(new_high_score(score)) init_score_entry();
			else {
				state = HIGH_SCORE_DISPLAY;
				state_timeout = 400;
			}
			break;
		case HIGH_SCORE_DISPLAY:
			state = TITLE_PAGE;
			state_timeout = 600.0;
			fadetimer = 0.0;
			break;
		case HIGH_SCORE_ENTRY:
			break;
		case TITLE_PAGE:
			state = HIGH_SCORE_DISPLAY;
			state_timeout = 200.0;
			break;
	}
}

void
gameloop() {
	SDL_Event e;
	Uint8 *keystate;
	float tmp;

	for(;;) {
		ms_frame = SDL_GetTicks() - ms_end;
		ms_end += ms_frame;
		if(ms_frame > 50) {
			ms_frame = 50;
		}
		t_frame = gamespeed * ms_frame / 50;
		frames++;

		if(opt_autopilot) {
			autopilot(t_frame);
		}

		while(SDL_PollEvent(&e)) {
			switch(e.type) {
				case SDL_QUIT: return;
				case SDL_KEYDOWN:
					if(state == HIGH_SCORE_ENTRY) {
						if(!process_score_input(&e.key.keysym)) {
							// Write the high score table to the file
							write_high_score_table();
							// continue to display the scores briefly
							state = HIGH_SCORE_DISPLAY;
							state_timeout = 200;
							play_tune(TUNE_TITLE_PAGE);
						}
					} else if(e.key.keysym.sym == SDLK_q) {
						return;
					}

					if(e.key.keysym.sym == SDLK_ESCAPE) {
						return;
					}
					break;
			}
		}
		keystate = SDL_GetKeyState(NULL);
		if(opt_autopilot) {
			autopilot_fix_keystates(keystate);
		}

		if(state == GAMEPLAY) {
			if(!paused) {
				score += ms_frame;
				
				if(keystate[SDLK_LEFT]  || keystate[SDLK_h]) { ship.dx -= THRUSTER_STRENGTH*t_frame; ship.jets |= 1<<0;}
				if(keystate[SDLK_DOWN]  || keystate[SDLK_t]) { ship.dy += THRUSTER_STRENGTH*t_frame; ship.jets |= 1<<1;}
				if(keystate[SDLK_RIGHT] || keystate[SDLK_n]) { ship.dx += THRUSTER_STRENGTH*t_frame; ship.jets |= 1<<2;}
				if(keystate[SDLK_UP]    || keystate[SDLK_c]) { ship.dy -= THRUSTER_STRENGTH*t_frame; ship.jets |= 1<<3;}
				if(ship.jets) {
					ship.dx = fconstrain2(ship.dx, -50, 50);
					ship.dy = fconstrain2(ship.dy, -50, 50);
				}
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

		if(!paused) {
			update_state();

			// SCROLLING
			tmp = (ship.y+ship.h/2 + ship.dy*t_frame - YSCROLLTO)/25 + (ship.dy-screendy);
			screendy += tmp * t_frame/12;
			tmp = (ship.x+ship.w/2 + ship.dx*t_frame - XSCROLLTO)/25 + (ship.dx-screendx);
			screendx += tmp * t_frame/12;
			// taper off so we don't hit the barrier abruptly.
			// (if we would hit in < 2 seconds, adjust to 2 seconds).
			if(dist_ahead + (screendx - BARRIER_SPEED)*TO_TICKS(2) < 0)
				screendx = BARRIER_SPEED - (dist_ahead/TO_TICKS(2));
			dist_ahead += (screendx - BARRIER_SPEED)*t_frame;
			if(MAX_DIST_AHEAD >= 0) dist_ahead = min(dist_ahead, MAX_DIST_AHEAD);

			move_sprites();
			move_dots();
			move_dust();

			new_rocks();

			// BOUNCE off left or right edge of screen
			if(ship.x < 0 || ship.x+ship.w > XSIZE) {
				ship.x -= (ship.dx-screendx)*t_frame;
				ship.dx = screendx - (ship.dx-screendx)*BOUNCINESS;
				ship.x = fconstrain(ship.x, XSIZE - ship.w);
			}

			// BOUNCE off top or bottom of screen
			if(ship.y < 0 || ship.y+ship.h > YSIZE) {
				ship.y -= (ship.dy-screendy)*t_frame;
				ship.dy = screendy - (ship.dy-screendy)*BOUNCINESS;
				ship.y = fconstrain(ship.y, YSIZE - ship.h);
			}

			new_engine_dots();

			collisions(); // must happen after ship bouncing because it puts pixels where the ship is (thus the ship must be on the screen)


			draw();

			// new game
			if((keystate[SDLK_SPACE] || keystate[SDLK_1] || keystate[SDLK_2])
			   && (state == HIGH_SCORE_DISPLAY
			       || state == TITLE_PAGE
			       || state == GAME_OVER)) {
				if(state == GAME_OVER && new_high_score(score))
					init_score_entry();
				else {
					if((keystate[SDLK_SPACE] && !initial_rocks) || keystate[SDLK_2]) {
						g_easy = 0;
						initial_rocks = NORMAL_I_ROCKS;
						final_rocks = NORMAL_F_ROCKS;
						if(gamespeed == EASY_GAMESPEED)
							gamespeed = NORMAL_GAMESPEED;
					} else if(keystate[SDLK_1]) {
						g_easy = 1;
						initial_rocks = EASY_I_ROCKS;
						final_rocks = EASY_F_ROCKS;
						gamespeed = EASY_GAMESPEED;
					}
					reset_sprites();
					reset_rocks();
					screendx = BARRIER_SPEED; screendy = 0;

					ship.x = XSIZE/2.2; ship.y = YSIZE/2 - ship.w/2;
					ship.dx = screendx; ship.dy = screendy;
					ship.lives = 4;
					ship.flags = MOVE|DRAW|COLLIDE;
					add_sprite(SPRITE(&ship));

					score = 0;

					state = GAMEPLAY;
					play_tune(TUNE_GAMEPLAY);
				}
			}

			ship.jets = 0;
		}

		if(state == TITLE_PAGE && keystate[SDLK_h]) {
			state = HIGH_SCORE_DISPLAY;
			state_timeout = 400;
		}
	}
}

int
main(int argc, char **argv) {
	if(!parse_opts(argc, argv)) return 1;

	if(init()) {
		printf ("vor: SDL error: '%s'\n",initerror);
		return 1;
	}

	start = SDL_GetTicks();
	frames = 0;
	gameloop();
	end = SDL_GetTicks();
	// printf("%ld frames in %ld ms, %.2f fps.\n", frames, end-start, frames * 1000.0 / (end-start));

	return 0;
}
