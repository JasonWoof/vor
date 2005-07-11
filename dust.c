#include <SDL.h>
#include <math.h>

#include "config.h"
#include "globals.h"
#include "dust.h"
#include "mt.h"

struct dust_mote {
	float x,y,z;
	Uint16 color;
};

struct dust_mote motes[N_DUST_MOTES];

void
init_dust(void)
{
	int i, b;
	for(i=0; i<N_DUST_MOTES; i++) {
		motes[i].x = frnd()*(XSIZE-5);
		motes[i].y = frnd()*(YSIZE-5);
		motes[i].z = MAX_DUST_DEPTH*sqrt(frnd());
		b = (MAX_DUST_DEPTH - motes[i].z) * 255.0 / MAX_DUST_DEPTH;
		motes[i].color = SDL_MapRGB(surf_screen->format, b, b, b);
	}
}

void
move_dust(void)
{
	int i;
	for(i=0; i<N_DUST_MOTES; i++) {
		motes[i].x -= xscroll / (1.3 + motes[i].z);
		motes[i].y -= yscroll / (1.3 + motes[i].z);
		if(motes[i].y >= XSIZE) motes[i].x -= XSIZE;
		else if(motes[i].x < 0) motes[i].x += XSIZE;
		if(motes[i].y > YSIZE) motes[i].y -= YSIZE;
		else if(motes[i].y < 0) motes[i].y += YSIZE;
	}
}

void
draw_dust(SDL_Surface *s)
{
	int i;
	uint16_t *pixels = s->pixels;
	for(i=0; i<N_DUST_MOTES; i++) {
		pixels[s->pitch/2*(int)motes[i].y + (int)motes[i].x] = motes[i].color;
	}
}