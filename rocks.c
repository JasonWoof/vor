#include <SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "file.h"
#include "globals.h"
#include "mt.h"
#include "rocks.h"
#include "sprite.h"

static struct rock rocks[MAXROCKS];
static struct rock prototypes[NROCKS];

// timers for rock generation.
static float rtimers[4];

uint32_t nrocks = NORMAL_I_ROCKS;
uint32_t initial_rocks = NORMAL_I_ROCKS;
uint32_t final_rocks = NORMAL_F_ROCKS;
float nrocks_timer = 0;
float nrocks_inc_ticks = 2*60*20/(NORMAL_F_ROCKS-NORMAL_I_ROCKS);

// constants for rock generation.
#define KH (32*20)  // 32 s for a speed=1 rock to cross the screen horizontally.
#define KV (24*20)  // 24 s for a speed=1 rock to cross the screen vertically.
#define RDX 2.5  // range for rock dx values (+/-)
#define RDY 2.5  // range for rock dy values (+/-)

void
reset_rocks(void)
{
	nrocks = initial_rocks;
	nrocks_inc_ticks = 2*60*20/(final_rocks-initial_rocks);
	nrocks_timer = 0;
}

#define ROCK_LEN sizeof("rockXX.png")

void
load_rocks(void)
{
	int i;
	char a[ROCK_LEN];

	for(i=0; i<NROCKS; i++) {
		snprintf(a, ROCK_LEN, "rock%02d.png", i);
		load_sprite(SPRITE(&prototypes[i]), a);
		prototypes[i].sprite_type = ROCK;
		prototypes[i].flags = MOVE|DRAW|COLLIDE;
	}

	memset(rocks, 0, MAXROCKS*sizeof(struct rock));

	for(i=1; i<MAXROCKS; i++) rocks[i].next = &rocks[i-1];
	free_sprites[ROCK] = SPRITE(&rocks[MAXROCKS-1]);

	reset_rocks();
}

enum { LEFT, RIGHT, TOP, BOTTOM };


// compute the number of rocks/tick that should be coming from each side,
// and the speed ranges of rocks coming from each side
void
rock_sides(float *ti, float *speed_min, float *speed_max)
{
	float dx0,dx1, dy0,dy1;
	float hfactor, vfactor;
	int i;

	for(i=0; i<4; i++) ti[i] = 0;
	for(i=0; i<4; i++) speed_min[i] = 0;
	for(i=0; i<4; i++) speed_max[i] = 0;
	hfactor = (float)nrocks/KH; vfactor = (float)nrocks/KV;

	dx0 = -RDX - screendx; dx1 = RDX - screendx;
	dy0 = -RDY - screendy; dy1 = RDY - screendy;

	if(dx0 < 0) {
		speed_max[RIGHT] = -dx0;
		if(dx1 < 0) {
			// Rocks moving left only. So the RIGHT side of the screen
			speed_min[RIGHT] = -dx1;
			ti[RIGHT] = -(dx0+dx1)/2;
		} else {
			// Rocks moving left and right
			speed_max[LEFT] = dx1;
			ti[RIGHT] = -dx0/2;
			ti[LEFT] = dx1/2;
		}
	} else {
		// Rocks moving right only. So the LEFT side of the screen
		speed_min[LEFT] = dx0;
		speed_max[LEFT] = dx1;
		ti[LEFT] = (dx0+dx1)/2;
	}
	ti[LEFT] *= hfactor;
	ti[RIGHT] *= hfactor;

	if(dy0 < 0) {
		speed_max[BOTTOM] = -dy0;
		if(dy1 < 0) {
			// Rocks moving up only. So the BOTTOM of the screen
			speed_min[BOTTOM] = -dy1;
			ti[BOTTOM] = -(dy0+dy1)/2;
		} else {
			// Rocks moving up and down
			speed_max[TOP] = dy1;
			ti[BOTTOM] = -dy0/2;
			ti[TOP] = dy1/2;
		}
	} else {
		// Rocks moving down only. so the TOP of the screen
		speed_min[TOP] = dy0;
		speed_max[TOP] = dy1;
		ti[TOP] = (dy0+dy1)/2;
	}
	ti[TOP] *= vfactor;
	ti[BOTTOM] *= vfactor;
}

float
weighted_rnd_range(float min, float max) {
	return sqrt(min * min + frnd() * (max * max - min * min));
}

void
new_rocks(void)
{
	int i, type;
	struct rock *r;
	float ti[4];
	float rmin[4];
	float rmax[4];

	if(nrocks < final_rocks) {
		nrocks_timer += t_frame;
		if(nrocks_timer >= nrocks_inc_ticks) {
			nrocks_timer -= nrocks_inc_ticks;
			nrocks++;
		}
	}

	rock_sides(ti, rmin, rmax);

	// increment timers
	for(i=0; i<4; i++) rtimers[i] += ti[i]*t_frame;

	// generate rocks
	for(i=0; i<4; i++) {
		while(rtimers[i] >= 1) {
			rtimers[i] -= 1;
			if(!free_sprites[ROCK]) return;  // sorry, we ran out of rocks!
			r = (struct rock *) remove_sprite(&free_sprites[ROCK]);
			type = urnd() % NROCKS;
			*r = prototypes[type];
			r->type = type;
			switch(i) {
				case RIGHT:
					r->x = XSIZE;
					r->y = frnd()*(YSIZE + r->image->h);

					r->dx = -weighted_rnd_range(rmin[i], rmax[i]) + screendx;
					r->dy = RDY*crnd();
					break;
				case LEFT:
					r->x = -r->image->w;
					r->y = frnd()*(YSIZE + r->image->h);

					r->dx = weighted_rnd_range(rmin[i], rmax[i]) + screendx;
					r->dy = RDY*crnd();
					break;
				case BOTTOM:
					r->x = (frnd()*(XSIZE + r->image->w)) - r->image->w;
					r->y = YSIZE;

					r->dx = RDX*crnd();
					r->dy = -weighted_rnd_range(rmin[i], rmax[i]) + screendy;
					break;
				case TOP:
					r->x = (frnd() * (XSIZE + r->image->w)) - r->image->w;
					r->y = -r->image->h;

					r->dx = RDX*crnd();
					r->dy = weighted_rnd_range(rmin[i], rmax[i]) + screendy;
					break;
			}
			add_sprite(SPRITE(r));
		}
	}
}


void
draw_rocks(void)
{
	int i;
	for(i=0; i<MAXROCKS; i++) draw_sprite(SPRITE(&rocks[i]));
}
