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

#undef DEBUG

extern int font_height;
void clearBuffer();

// includes {{{
#include "config.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "SFont.h"
// }}}
// constants {{{
// }}}
// macros {{{
#define CONDERROR(a) if((a)) {initerror = strdup(SDL_GetError());return 1;}
#define NULLERROR(a) CONDERROR((a) == NULL)
// }}}

// ************************************* STRUCTS
struct rock_struct {
	// Array of black pixel coordinates. This is scanned 
	// every frame to see if it's still black, and as
	// soon as it isn't we BLOW UP
	float x,y,xvel,yvel;
	int active;
	SDL_Surface *image;
	int type_number;
	float heat;
}; 
struct black_point_struct {
	int x,y;
};
struct bangdots {
	// Bang dots have the same colour as shield dots.
	// Bang dots get darker as they age.
	// Some are coloured the same as the ex-ship.
	float x,y,dx,dy;
	Uint16 c; // when zero, use heatcolor[bangdotlife]
	float life;	// When reduced to 0, set active = 0
	int active;
	float decay;// Amount by which to reduce life each time dot is drawn
};
struct enginedots {
	// Engine dots stream out the back of the ship, getting darker as they go.
	int active;
	float x,y,dx,dy;
	// The life of an engine dot 
	// is a number starting at between 0 and 50 and counting backward.
	float life;	// When reduced to 0, set active = 0
};
struct spacedot {
	// Space dots are harmless background items
	// All are active. When one falls off the edge, another is created at the start.
	float x,y,dx;
	Uint16 color;
};
// High score table {{{
struct highscore {
	int score;
	char *name;
	int allocated;
} high[] = {
	{13000,"Pad",0},
	{12500,"Pad",0},
	{6500,"Pad",0},
	{5000,"Pad",0},
	{3000,"Pad",0},
	{2500,"Pad",0},
	{2000,"Pad",0},
	{1500,"Pad",0}
};
// }}}

// ************************************* VARS
// SDL_Surface global variables {{{
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
	*surf_deadrock[NROCKS],	// THE DEAD ROCKS
	*surf_font_big;	// The big font
// }}}
// Structure global variables {{{
struct enginedots edot[MAXENGINEDOTS], *dotptr = edot;
struct rock_struct rock[MAXROCKS], *rockptr = rock;
struct black_point_struct black_point[MAXBLACKPOINTS], *blackptr = black_point;
struct bangdots bdot[MAXBANGDOTS], *bdotptr = bdot;
struct spacedot sdot[MAXSPACEDOTS];
// }}}
// Other global variables {{{
char topline[1024];
char *initerror = "";
char name[1024], debug1[1024];

float xship,yship = 240.0;	// X position, 0..XSIZE
float xvel,yvel;	// Change in X position per tick.
float rockrate,rockspeed;
float movementrate;
float yscroll;

int nships,score,initticks,ticks_since_last, last_ticks;
int gameover;
int countdown = 0;
int maneuver = 0;
int oss_sound_flag = 0;
int tail_plume = 0; // display big engine at the back?
int friction = 0;	// should there be friction?
int scorerank;
float fadetimer = 0,faderate;

int pausedown = 0,paused = 0;

// bangdot start (bd1) and end (bd2) position:
int bd1 = 0, bd2 = 0;

int xoffset[NROCKS][MAXROCKHEIGHT];

enum states {
	TITLE_PAGE,
	GAMEPLAY,
	DEAD_PAUSE,
	GAME_OVER,
	HIGH_SCORE_ENTRY,
	HIGH_SCORE_DISPLAY,
	DEMO
};
enum states state = TITLE_PAGE;
float state_timeout = 600.0;

const int fakesin[] = {0,1,0,-1};
const int fakecos[] = {1,0,-1,0};
#define NSEQUENCE 2
char *sequence[] = {
	"Press SPACE to start",
	"http://qualdan.com/vor/"
};

int bangdotlife, nbangdots;
Uint16 heatcolor[W*3];

char *data_dir;
extern char *optarg;
extern int optind, opterr, optopt;
// }}}

float dist_sq(float x1, float y1, float x2, float y2)
{
	return (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
}

// ************************************* FUNCS

FILE *hs_fopen(char *mode) {
	FILE *f;
	mode_t mask;
	mask = umask(0111);
	if(f = fopen("/usr/share/vor/.highscore",mode)) {
		umask(mask);
		return f;
	}
	else {
		char s[1024];
		umask(0177);
		sprintf(s,"%s/.vor-high",getenv("HOME"));
		if(f = fopen(s,mode)) {
			umask(mask);
			return f;
		}
		else {
			umask(mask);
			return 0;
		}
	}
}
void read_high_score_table() {
	FILE *f;
	int i;
	if(f = hs_fopen("r")) {
		// If the file exists, read from it
		for(i = 0; i<8; i++) {
			char s[1024];
			int highscore;
			if(fscanf (f, "%d %[^\n]", &highscore, s) != 2) {
				break;
			}
			if(high[i].allocated) {
				free(high[i].name);
			}
			high[i].name = strdup(s);
			high[i].score = highscore;
			high[i].allocated = 1;
		}
		fclose(f);
	}
}
void write_high_score_table() {
	FILE *f;
	int i;
	if(f = hs_fopen("w")) {
		// If the file exists, write to it
		for(i = 0; i<8; i++) {
			fprintf (f, "%d %s\n", high[i].score, high[i].name);
		}
		fclose(f);
	}
}
void snprintscore(char *s, size_t n, int score) {
	int min = score/60000;
	int sec = score/1000%60;
	int tenths = score%1000/100;
	if(min) {
		snprintf(s, n, "%2d:%.2d.%d", min, sec, tenths);
	} else {
		snprintf(s, n, " %2d.%d", sec, tenths);
	}
}
float rnd() {
	return (float)random()/(float)RAND_MAX;
}
void init_engine_dots() {
	int i;
	for(i = 0; i<MAXENGINEDOTS; i++) {
		edot[i].active = 0;
	}
}
void init_space_dots() {
	int i,intensity;
	for(i = 0; i<MAXSPACEDOTS; i++) {
		float r;

		sdot[i].x = rnd()*(XSIZE-5);
		sdot[i].y = rnd()*(YSIZE-5);

		r = rnd()*rnd();

		sdot[i].dx = -r*4;
		// -1/((1-r) + .3);
		intensity = (int)(r*180 + 70);
		sdot[i].color = SDL_MapRGB(surf_screen->format,intensity,intensity,intensity);

	}
}

int makebangdots(int xbang, int ybang, int xvel, int yvel, SDL_Surface *s, int power) {

	// TODO - stop generating dots after a certain amount of time has passed, to cope with slower CPUs.
	// TODO - generate and display dots in a circular buffer

	int i,x,y,n,endcount;
	Uint16 *rawpixel,c;
	double theta,r,dx,dy;
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
			if(c && c != SDL_MapRGB(s->format,0,255,0)) {

				theta = rnd()*M_PI*2;

				r = 1-(rnd()*rnd());

				bdot[bd2].dx = (power/50.0)*45.0*cos(theta)*r + xvel;
				bdot[bd2].dy = (power/50.0)*45.0*sin(theta)*r + yvel;
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
exitloop:

	SDL_UnlockSurface(s);

}

void draw_bang_dots(SDL_Surface *s) {
	int i;
	int first_i, last_i;
	Uint16 *rawpixel;
	rawpixel = (Uint16 *) s->pixels;

	first_i = -1;

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
			bdot[i].x += bdot[i].dx*movementrate;
			bdot[i].y += bdot[i].dy*movementrate + yscroll;

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


void draw_space_dots(SDL_Surface *s) {
	int i;
	Uint16 *rawpixel;
	rawpixel = (Uint16 *) s->pixels;

	for(i = 0; i<MAXSPACEDOTS; i++) {
		if(sdot[i].y<0) {
			sdot[i].y = 0;
		}
		rawpixel[(int)(s->pitch/2*(int)sdot[i].y) + (int)(sdot[i].x)] = sdot[i].color;
		sdot[i].x += sdot[i].dx*movementrate;
		sdot[i].y += yscroll;
		if(sdot[i].y > YSIZE) {
			sdot[i].y -= YSIZE;
		} else if(sdot[i].y < 0) {
			sdot[i].y += YSIZE;
		}
		if(sdot[i].x<0) {
			sdot[i].x = XSIZE;
		}
	}
}

void draw_engine_dots(SDL_Surface *s) {
	int i;
	Uint16 *rawpixel;
	rawpixel = (Uint16 *) s->pixels;

	for(i = 0; i<MAXENGINEDOTS; i++) {
		if(edot[i].active) {
			edot[i].x += edot[i].dx*movementrate;
			edot[i].y += edot[i].dy*movementrate + yscroll;
			if((edot[i].life -= movementrate*3)<0 || edot[i].y<0 || edot[i].y>YSIZE) {
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

void create_engine_dots(int newdots) {
	int i;
	double theta,r,dx,dy;

	if(!tail_plume) return;

	if(state == GAMEPLAY) {
		for(i = 0; i<newdots*movementrate; i++) {
			if(dotptr->active == 0) {
				theta = rnd()*M_PI*2;
				r = rnd();
				dx = cos(theta)*r;
				dy = sin(theta)*r;

				dotptr->active = 1;
				dotptr->x = xship + surf_ship->w/2-14;
				dotptr->y = yship + surf_ship->h/2 + (rnd()-0.5)*5-1;
				dotptr->dx = 10*(dx-1.5) + xvel;
				dotptr->dy = 1*dy + yvel;
				dotptr->life = 45 + rnd(1)*5;

				dotptr++;
				if(dotptr-edot >= MAXENGINEDOTS) {
					dotptr = edot;
				}
			}
		}
	}
}

void create_engine_dots2(int newdots, int m) {
	int i;
	double theta, theta2, dx, dy, adx, ady;

	// Don't create fresh engine dots when
	// the game is not being played and a demo is not beng shown
	if(state != GAMEPLAY && state != DEMO) return;

	for(i = 0; i<newdots; i++) {
		if(dotptr->active == 0) {
			theta = rnd()*M_PI*2;
			theta2 = rnd()*M_PI*2;

			dx = cos(theta) * fabs(cos(theta2));
			dy = sin(theta) * fabs(cos(theta2));
			adx = fabs(dx);
			ady = fabs(dy);


			dotptr->active = 1;
			dotptr->x = xship + surf_ship->w/2 + (rnd()-0.5)*3;
			dotptr->y = yship + surf_ship->h/2 + (rnd()-0.5)*3;

			switch(m) {
				case 0:
					dotptr->x -= 14;
					dotptr->dx = -20*adx + xvel;
					dotptr->dy = 2*dy + yvel;
					dotptr->life = 60 * adx;
				break;
				case 1:
					dotptr->dx = 2*dx + xvel;
					dotptr->dy = -20*ady + yvel;
					dotptr->life = 60 * ady;
				break;
				case 2:
					dotptr->x += 14;
					dotptr->dx = 20*adx + xvel;
					dotptr->dy = 2*dy + yvel;
					dotptr->life = 60 * adx;
				break;
				case 3:
					dotptr->dx = 2*dx + xvel;
					dotptr->dy = 20*ady + yvel;
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

int drawdots(SDL_Surface *s) {
	int m, scorepos, n;

	SDL_LockSurface(s);
	// Draw the background stars aka space dots
	draw_space_dots(s);

	// Draw the score when playing the game or whn the game is freshly over
	if(1 || state == GAMEPLAY || state == DEAD_PAUSE || state == GAME_OVER ) {
		SDL_UnlockSurface(s);

		scorepos = XSIZE-250;
		n = snprintf(topline, 50, "Time: ");
		snprintscore(topline + n, 50-n, score);
		PutString(s,scorepos,0,topline);

		SDL_LockSurface(s);
	}

	// Draw all the engine dots
	draw_engine_dots(s);

	// Create more engine dots comin out da back
	if(!gameover)
	create_engine_dots(200);

	// Create engine dots out the side we're moving from
	for(m = 0; m<4; m++) {
		if(maneuver & 1<<m) { // 'maneuver' is a bit field
			create_engine_dots2(80,m);
		}
	}

	// Draw all outstanding bang dots
	//if(bangdotlife-- > 0) 
	draw_bang_dots(s);

	SDL_UnlockSurface(s);
}

char * load_file(char *s) {
	static char retval[1024];
	snprintf(retval, 1024, "%s/%s", data_dir, s);
	return retval;
}


int missing(char *dirname) {
	struct stat buf;
	stat(dirname, &buf);
	return (!S_ISDIR(buf.st_mode));
}

int init(int fullscreen) {

	int i,j;
	SDL_Surface *temp;
	Uint16 *raw_pixels;
	Uint32 flag;

	read_high_score_table();

	// Where are our data files?
	// default: ./data
	// second alternative: RD_DATADIR
	// final alternative: /usr/share/vor
	data_dir = strdup("./data");
	if(missing(data_dir)) {
		char *env;
		env = getenv("RD_DATADIR");
		if(env != NULL) {
			data_dir = strdup(env);
			if(missing(data_dir)) {
				fprintf (stderr,"Cannot find data directory $RD_DATADIR\n");
				exit(-1);
			}
		} else {
			data_dir = strdup("/usr/share/vor");
			if(missing(data_dir)) {
				fprintf (stderr,"Cannot find data in %s\n", data_dir);
				exit(-2);
			}
		}
	}

	if(oss_sound_flag) {

	// Initialise SDL with audio and video
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		oss_sound_flag = 0;
		printf ("Can't open sound, starting without it\n");
		atexit(SDL_Quit);
	} else {
		atexit(SDL_Quit);
		atexit(SDL_CloseAudio);
		oss_sound_flag = init_sound();
	}

	} else {
		// Initialise with video only
		CONDERROR(SDL_Init(SDL_INIT_VIDEO) != 0);
		atexit(SDL_Quit);
	}

	if(oss_sound_flag)
	play_tune(0);

	// Attempt to get the required video size
	flag = SDL_DOUBLEBUF | SDL_HWSURFACE;
	if(fullscreen) flag |= SDL_FULLSCREEN;
	surf_screen = SDL_SetVideoMode(XSIZE,YSIZE,16,flag);

	// Set the title bar text
	SDL_WM_SetCaption("Rock Dodgers", "rockdodgers");

	NULLERROR(surf_screen);

	// Set the heat color from the range 0 (cold) to 300 (blue-white)
	for(i = 0; i<W*3; i++) {
		heatcolor[i] = SDL_MapRGB(
			surf_screen->format,
			(i<W)?(i*M/W):(M),(i<W)?0:(i<2*W)?((i-W)*M/W):M,(i<2*W)?0:((i-W)*M/W) // Got that?
		);
	}

	// Load the banners
	NULLERROR(temp = IMG_Load(load_file("banners/variations.png")));
	NULLERROR(surf_b_variations = SDL_DisplayFormat(temp));

	NULLERROR(temp = IMG_Load(load_file("banners/on.png")));
	NULLERROR(surf_b_on = SDL_DisplayFormat(temp));

	NULLERROR(temp = IMG_Load(load_file("banners/rockdodger.png")));
	NULLERROR(surf_b_rockdodger = SDL_DisplayFormat(temp));

	NULLERROR(temp = IMG_Load(load_file("banners/game.png")));
	NULLERROR(surf_b_game = SDL_DisplayFormat(temp));

	NULLERROR(temp = IMG_Load(load_file("banners/over.png")));
	NULLERROR(surf_b_over = SDL_DisplayFormat(temp));

	surf_font_big = IMG_Load(load_file(BIG_FONT_FILE));
	InitFont(surf_font_big);

	// Load the spaceship graphic.
	NULLERROR(temp = IMG_Load(load_file("sprites/ship.png")));
	NULLERROR(surf_ship = SDL_DisplayFormat(temp));

	// Load the life indicator (small ship) graphic.
	NULLERROR(temp = IMG_Load(load_file("indicators/life.png")));
	NULLERROR(surf_life = SDL_DisplayFormat(temp));

	// Create the array of black points;
	SDL_LockSurface(surf_ship);
	raw_pixels = (Uint16 *) surf_ship->pixels;
	for(i = 0; i<surf_ship->w; i++) {
		for(j = 0; j<surf_ship->h; j++) {
			if(raw_pixels[j*(surf_ship->pitch)/2 + i] == 0) {
				blackptr->x = i;
				blackptr->y = j;
				blackptr++;
			}
		}
	}

	SDL_UnlockSurface(surf_ship);

	init_engine_dots();
	init_space_dots();

	// Load all our lovely rocks
	for(i = 0; i<NROCKS; i++) {
		char a[100];

		sprintf(a,load_file("sprites/rock%d.png"),i);
		NULLERROR(temp = IMG_Load(a));
		NULLERROR(surf_rock[i] = SDL_DisplayFormat(temp));

		sprintf(a,load_file("sprites/deadrock%d.png"),i);
		NULLERROR(temp = IMG_Load(a));
		NULLERROR(surf_deadrock[i] = SDL_DisplayFormat(temp));
	}

	// Remove the mouse cursor
#ifdef SDL_DISABLE
	SDL_ShowCursor(SDL_DISABLE);
#endif

	return 0;
}
int draw() {
	int i,n;
	SDL_Rect src,dest;
	struct black_point_struct *p;
	Uint16 *raw_pixels;
	int bang, offset, x;
	char *text;
	float fadegame,fadeover;

	char *statedisplay, buf[1024];
	
	bang = 0;

	src.x = 0;
	src.y = 0;
	dest.x = 0;
	dest.y = 0;

	// Draw a fully black background
	SDL_FillRect(surf_screen,NULL,0);


#ifdef DEBUG
	// DEBUG {{{
	// Show the current state
	switch (state) {
		case TITLE_PAGE:
			statedisplay = "title_page";
		break;
		case GAMEPLAY:
			statedisplay = "gameplay";
		break;
		case DEAD_PAUSE:
			statedisplay = "dead_pause";
		break;
		case GAME_OVER:
			statedisplay = "game_over";
		break;
		case HIGH_SCORE_ENTRY:
			statedisplay = "high_score_entry";
		break;
		case HIGH_SCORE_DISPLAY:
			statedisplay = "high_score_display";
		break;
		case DEMO:
			statedisplay = "demo";
		break;
	}
	snprintf(buf,1024, "mode = %s", statedisplay);
	PutString(surf_screen,0,YSIZE-50,buf);
	// }}}
#endif
	
	// Draw the background dots
	drawdots(surf_screen);

	// Draw ship
	if(!gameover && (state == GAMEPLAY || state == DEMO) ) {
		src.w = surf_ship->w;
		src.h = surf_ship->h;
		dest.w = src.w;
		dest.h = src.h;
		dest.x = (int)xship;
		dest.y = (int)yship;
		SDL_BlitSurface(surf_ship,&src,surf_screen,&dest);
	}

	// Draw all the rocks, in all states
	for(i = 0; i<MAXROCKS; i++) {
		if(rock[i].active) {

			src.w = rock[i].image->w;
			src.h = rock[i].image->h;
			dest.w = src.w;
			dest.h = src.h;
			dest.x = (int) rock[i].x;
			dest.y = (int) rock[i].y;

			// Draw the rock
			SDL_BlitSurface(rock[i].image,&src,surf_screen,&dest);

			// Draw the heated part of the rock, in an alpha which reflects the
			// amount of heat in the rock.
			if(rock[i].heat>0) {
				SDL_Surface *deadrock;
				deadrock = surf_deadrock[rock[i].type_number];
				SDL_SetAlpha(deadrock,SDL_SRCALPHA,rock[i].heat*255/rock[i].image->h);
				dest.x = (int) rock[i].x; // kludge
				SDL_BlitSurface(deadrock,&src,surf_screen,&dest);
				if(rnd()<0.3) {
					rock[i].heat -= movementrate;
				}
			}

			// If the rock is heated past a certain point, the water content of
			// the rock flashes to steam, releasing enough energy to destroy
			// the rock in spectacular fashion.
			if(rock[i].heat>rock[i].image->h) {
				rock[i].active = 0;
				play_sound(1 + (int)(rnd()*3));
				makebangdots(rock[i].x,rock[i].y,rock[i].xvel,rock[i].yvel,rock[i].image,10);
			}

		}
	}

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

			src.w = surf_b_game->w;
			src.h = surf_b_game->h;
			dest.w = src.w;
			dest.h = src.h;
			dest.x = (XSIZE-src.w)/2;
			dest.y = (YSIZE-src.h)/2-40;
			SDL_SetAlpha(surf_b_game, SDL_SRCALPHA, (int)(fadegame*(200 + 55*cos(fadetimer += movementrate/1.0))));
			SDL_BlitSurface(surf_b_game,&src,surf_screen,&dest);

			src.w = surf_b_over->w;
			src.h = surf_b_over->h;
			dest.w = src.w;
			dest.h = src.h;
			dest.x = (XSIZE-src.w)/2;
			dest.y = (YSIZE-src.h)/2 + 40;
			SDL_SetAlpha(surf_b_over, SDL_SRCALPHA, (int)(fadeover*(200 + 55*sin(fadetimer))));
			SDL_BlitSurface(surf_b_over,&src,surf_screen,&dest);
		break;

		case TITLE_PAGE:

			src.w = surf_b_variations->w;
			src.h = surf_b_variations->h;
			dest.w = src.w;
			dest.h = src.h;
			dest.x = (XSIZE-src.w)/2 + cos(fadetimer/6.5)*10;
			dest.y = (YSIZE/2-src.h)/2 + sin(fadetimer/5.0)*10;
			SDL_SetAlpha(surf_b_variations, SDL_SRCALPHA, (int)(200 + 55*sin(fadetimer += movementrate/2.0)));
			SDL_BlitSurface(surf_b_variations,&src,surf_screen,&dest);

			src.w = surf_b_on->w;
			src.h = surf_b_on->h;
			dest.w = src.w;
			dest.h = src.h;
			dest.x = (XSIZE-src.w)/2 + cos((fadetimer + 1.0)/6.5)*10;
			dest.y = (YSIZE/2-src.h)/2 + surf_b_variations->h + 20 + sin((fadetimer + 1.0)/5.0)*10;
			SDL_SetAlpha(surf_b_on, SDL_SRCALPHA, (int)(200 + 55*sin(fadetimer-1.0)));
			SDL_BlitSurface(surf_b_on,&src,surf_screen,&dest);

			src.w = surf_b_rockdodger->w;
			src.h = surf_b_rockdodger->h;
			dest.w = src.w;
			dest.h = src.h;
			dest.x = (XSIZE-src.w)/2 + cos((fadetimer + 2.0)/6.5)*10;
			dest.y = (YSIZE/2-src.h)/2 + surf_b_variations->h + surf_b_on->h + 40 + sin((fadetimer + 2.0)/5)*10;
			SDL_SetAlpha(surf_b_rockdodger, SDL_SRCALPHA, (int)(200 + 55*sin(fadetimer-2.0)));
			SDL_BlitSurface(surf_b_rockdodger,&src,surf_screen,&dest);

			text = "Version " VERSION;
			x = (XSIZE-SFont_wide(text))/2 + sin(fadetimer/4.5)*10;
			PutString(surf_screen,x,YSIZE-50 + sin(fadetimer/2)*5,text);

			text = sequence[(int)(fadetimer/40)%NSEQUENCE];
			//text = "Press SPACE to start!";
			x = (XSIZE-SFont_wide(text))/2 + cos(fadetimer/4.5)*10;
			PutString(surf_screen,x,YSIZE-100 + cos(fadetimer/3)*5,text);
		break;

		case HIGH_SCORE_ENTRY:

			if(score >= high[7].score) {
				play_tune(2);
				if(SFont_Input (surf_screen, 330, 50 + (scorerank + 2)*font_height, 300, name)) {
					// Insert name into high score table

					// Lose the lowest name forever (loser!)
					//if(high[7].allocated)
					//	free(high[7].name);			// THIS WAS CRASHING SO I REMOVED IT

					// Insert new high score
					high[scorerank].score = score;
					high[scorerank].name = strdup(name);	// MEMORY NEVER FREED!
					high[scorerank].allocated = 1;
			
					// Set the global name string to "", ready for the next winner
					name[0] = 0;
			
					// Change state to briefly show high scores page
					state = HIGH_SCORE_DISPLAY;
					state_timeout = 200;

					// Write the high score table to the file
					write_high_score_table();
			
					// Play the title page tune
					play_tune(0);
				}
			} else {
				state = HIGH_SCORE_DISPLAY;
				state_timeout = 400;
			}
		// FALL THROUGH

		case HIGH_SCORE_DISPLAY:
			// Display de list o high scores mon.
			PutString(surf_screen,180,50,"High scores");
			for(i = 0; i<8; i++) {
				char s[1024];
				sprintf(s, "#%1d",i + 1);
				PutString(surf_screen, 150, 50 + (i + 2)*font_height,s);
				snprintscore(s, 1024, high[i].score);
				PutString(surf_screen, 200, 50 + (i + 2)*font_height,s);
				sprintf(s, "%3s", high[i].name);
				PutString(surf_screen, 330, 50 + (i + 2)*font_height,s);
			}

	}

	if(!gameover && state == GAMEPLAY) {
		SDL_LockSurface(surf_screen);
		raw_pixels = (Uint16 *) surf_screen->pixels;
		// Check that the black points on the ship are
		// still black, and not covered up by rocks.
		for(p = black_point; p<blackptr; p++) { 
			offset = surf_screen->pitch/2 * (p->y + (int)yship) + p->x + (int)xship;
			if(raw_pixels[offset]) {
				// Set the bang flag
				bang = 1;
			}
		}
		SDL_UnlockSurface(surf_screen);
	}

	// Draw all the little ships
	if(state == GAMEPLAY || state == DEAD_PAUSE || state == GAME_OVER)
	for(i = 0; i<nships-1; i++) {
		src.w = surf_life->w;
		src.h = surf_life->h;
		dest.w = src.w;
		dest.h = src.h;
		dest.x = (i + 1)*(src.w + 10);
		dest.y = 20;
		SDL_BlitSurface(surf_life,&src,surf_screen,&dest);
	}


	// Update the score
	/*
	n = SDL_GetTicks()-initticks;
	if(score)
	ticks_since_last = n-score;
	score = n;
	*/

	ticks_since_last = SDL_GetTicks()-last_ticks;
	last_ticks = SDL_GetTicks();
	if(ticks_since_last>200 || ticks_since_last<0) {
		movementrate = 0;
	}
	else {
		movementrate = ticks_since_last/50.0;
		if(state == GAMEPLAY) {
			score += ticks_since_last;
		}
	}

	// Update the surface
	SDL_Flip(surf_screen);


	return bang;
}
int gameloop() {
	int i = 0;
	Uint8 *keystate;


	for(;;) {
		if(!paused) {
			// Count down the game loop timer, and change state when it gets to zero or less;

			if((state_timeout -= movementrate*3) < 0) {
				switch(state) {
					case DEAD_PAUSE:
						// Create a new ship and start all over again
						state = GAMEPLAY;
						play_tune(1);
						xship = 10;
						yship = YSIZE/2;
						xvel = 3;
						yvel = 0;
						for(i = 0; i<MAXROCKS; i++ ) {
							if(dist_sq(xship, yship, rock[i].x, rock[i].y) < START_RAD_SQ) {
								rock[i].active = 0;
							}
						}
					break;
					case GAME_OVER:
						state = HIGH_SCORE_ENTRY;
						clearBuffer();
						name[0] = 0;
						state_timeout = 5.0e6;

						if(score >= high[7].score) {
							// Read the high score table from the storage file
							read_high_score_table();

							// Find ranking of this score, store as scorerank
							for(i = 0; i<8; i++) {
							if(high[i].score <= score) {
								scorerank = i;
								break;
							}
							}

							// Move all lower scores down a notch
							for(i = 7; i >= scorerank; i--)
							high[i] = high[i-1];

							// Insert blank high score
							high[scorerank].score = score;
							high[scorerank].name = "";
							high[scorerank].allocated = 0;
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
				}
			}

			if(--countdown <= 0 && (rnd()*100.0<(rockrate += 0.025))) {
				// Create a rock
				rockptr++;
				if(rockptr-rock >= MAXROCKS) {
					rockptr = rock;
				}
				if(!rockptr->active) {
					rockptr->x = (float)XSIZE;
					rockptr->xvel = -(rockspeed)*(1 + rnd());
					rockptr->yvel = rnd()-0.5;
					rockptr->type_number = random() % NROCKS;
					rockptr->heat = 0;
					rockptr->image = surf_rock[rockptr->type_number];// [random()%NROCKS];
					rockptr->active = 1;
					rockptr->y = rnd()*(YSIZE + rockptr->image->h);
				}
				if(movementrate>0.1) {
					countdown = (int)(ROCKRATE/movementrate);
				} else {
					countdown = 0;
				}
			}

			// FRICTION?
			if(friction) {
				xvel *= pow((double)0.9,(double)movementrate);
				yvel *= pow((double)0.9,(double)movementrate);
				// if(abs(xvel)<0.00001) xvel = 0;
				// if(abs(yvel)<0.00001) yvel = 0;
			}

			// INERTIA
			xship += xvel*movementrate;
			yship += yvel*movementrate;

			// SCROLLING
			yscroll = yship - (YSIZE / 2);
			yscroll /= -15;
			yscroll = yscroll*movementrate;
			yship += yscroll;
			
			// Move all the rocks
			for(i = 0; i<MAXROCKS; i++) if(rock[i].active) {
				rock[i].x += rock[i].xvel*movementrate;
				rock[i].y += rock[i].yvel*movementrate + yscroll;
			if(rock[i].y > YSIZE) {
				rock[i].y -= YSIZE;
				rock[i].y -= rock[i].image->w;
			} else if(rock[i].y < -rock[i].image->w) {
				rock[i].y += YSIZE;
				rock[i].y += rock[i].image->w;
			}
			if(rock[i].x<-32.0)
				rock[i].active = 0;
			}


			// BOUNCE X
			if(xship<0 || xship>XSIZE-surf_ship->w) {
				// BOUNCE from left and right wall
				xship -= xvel*movementrate;
				xvel *= -0.99;
			}

			// BOUNCE Y
			if(yship<0 || yship>YSIZE-surf_ship->h) {
				// BOUNCE from top and bottom wall
				yship -= yvel;
				yvel *= -0.99;
			}


			if(draw() && state == GAMEPLAY) {
				if(oss_sound_flag) {
					// Play the explosion sound
					play_sound(0);
				}
				makebangdots(xship,yship,xvel,yvel,surf_ship,30);
				if(--nships <= 0) {
					gameover = 1;
					state = GAME_OVER;
					state_timeout = 200.0;
					fadetimer = 0.0;
					faderate = movementrate;
				}
				else {
					state = DEAD_PAUSE;
					state_timeout = 50.0;

				}
			}

			SDL_PumpEvents();
			keystate = SDL_GetKeyState(NULL);

			if(state != HIGH_SCORE_ENTRY && (keystate[SDLK_q] || keystate[SDLK_ESCAPE])) {
				return 0;
			}

			if(keystate[SDLK_SPACE] && (state == HIGH_SCORE_DISPLAY || state == TITLE_PAGE || state == DEMO)) {

				for(i = 0; i<MAXROCKS; i++ ) {
					rock[i].active = 0;
				}

				rockrate = 54.0;
				rockspeed = 5.0;

				nships = 4;
				score = 0;

				state = GAMEPLAY;
				play_tune(1);

				xvel = -1;
				gameover = 0;
				yvel = 0;
				xship = 0;
				yship = YSIZE/2;

			}

			maneuver = 0;
		} else {
			SDL_PumpEvents();
			keystate = SDL_GetKeyState(NULL);
		}

		if(state == GAMEPLAY) {
			if(!gameover) {

				if(!paused) {
					if(keystate[SDLK_UP] | keystate[SDLK_c])		{ yvel -= 1.5*movementrate; maneuver |= 1<<3;}
					if(keystate[SDLK_DOWN] | keystate[SDLK_t])		{ yvel += 1.5*movementrate; maneuver |= 1<<1;}
					if(keystate[SDLK_LEFT] | keystate[SDLK_h])		{ xvel -= 1.5*movementrate; maneuver |= 1<<2;}
					if(keystate[SDLK_RIGHT] | keystate[SDLK_n])		{ xvel += 1.5*movementrate; maneuver |= 1;}
					if(keystate[SDLK_3])		{ SDL_SaveBMP(surf_screen, "snapshot.bmp"); }
				}

				if(keystate[SDLK_p] | keystate[SDLK_s]) {
					if(!pausedown) {
						paused = !paused;
						if(paused) {
							SDL_Rect src,dest;
							src.w = surf_b_variations->w;
							src.h = surf_b_variations->h;
							dest.w = src.w;
							dest.h = src.h;
							dest.x = (XSIZE-src.w)/2;
							dest.y = (YSIZE-src.h)/2;
							SDL_BlitSurface(surf_b_variations,&src,surf_screen,&dest);
							// Update the surface
							SDL_Flip(surf_screen);
						}
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
	}
}
main(int argc, char **argv) {
	int i, x, fullscreen;

	fullscreen = 0;
	tail_plume = 0;
	friction = 0;
	oss_sound_flag = 1;

	while ((x = getopt(argc,argv,"efhsp")) >= 0) {
		switch(x) {
			case 'e': // engine
				tail_plume = 1;
			break;
			case 'f': // fullscreen
				fullscreen = 1;
			break;
			case 'h': // help
				printf("Variations on RockDodger\n"
				       " -e big tail [E]ngine\n"
				       " -f [F]ull screen\n"
				       " -h this [H]elp message\n"
				       " -p original [P]hysics (friction)\n"
				       " -s [S]ilent (no sound)\n");
				exit(0);
			break;
			case 'p': // physics
				friction = 1;
			break;
			case 's': // silent
				oss_sound_flag = 0;
			break;
		}
	}

	if(init(fullscreen)) {
		printf ("ta: '%s'\n",initerror);
		return 1;
	}

	while(1) {
		for(i = 0; i<MAXROCKS; i++) {
			rock[i].active = 0;
		}
		rockrate = 54.0;
		rockspeed = 5.0;
		initticks = SDL_GetTicks();
		if(gameloop() == 0) {
			break;
		}
		printf ("score = %d\n",score);
		SDL_Delay(1000);
	}

	return 0;
}
