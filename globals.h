#ifndef VOR_GLOBALS_H
#define VOR_GLOBALS_H

#include <SDL.h>
#include <inttypes.h>
#include "font.h"

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

extern float t_frame;

extern font *g_font;

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
