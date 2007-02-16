#ifndef VOR_GLOBALS_H
#define VOR_GLOBALS_H

#include <SDL.h>
#include <inttypes.h>
#include "font.h"

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

// ************************************* VARS
// SDL_Surface global variables
extern SDL_Surface 
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

extern font *g_font;

// Structure global variables
extern struct enginedots edot[MAXENGINEDOTS], *dotptr;
extern struct bangdots bdot[MAXBANGDOTS], *bdotptr;

// Other global variables
extern char topline[1024];
extern char *initerror;

extern float screendx, screendy;

extern int score;
extern int g_easy;
extern float fadetimer, faderate;

extern int pausedown, paused;

// bangdot start (bd1) and end (bd2) position:
extern int bd1, bd2;

extern int bangdotlife, nbangdots;
extern Uint16 heatcolor[W*3];

extern char *data_dir;

extern uint32_t initial_rocks, final_rocks;

#endif // VOR_GLOBALS_H
