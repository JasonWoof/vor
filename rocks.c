#include <SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "file.h"
#include "globals.h"
#include "rocks.h"
#include "shape.h"

struct rock_struct {
	float x,y,dx,dy;
	int active;
	SDL_Surface *image;
	struct shape *shape;
	int type_number;
}; 

struct rock_struct rock[MAXROCKS], *rockptr = rock;

SDL_Surface *surf_rock[NROCKS];
struct shape rock_shapes[NROCKS];

// timers for rock generation.
float rtimers[4];

uint32_t nrocks;
uint32_t nrocks_timer;
uint32_t nrocks_inc_ticks = 2*60*1000/(F_ROCKS-I_ROCKS);

// constants for rock generation.
#define KH 32.0  // 32 s for a speed=1 rock to cross the screen horizontally.
#define KV 24.0  // 24 s for a speed=1 rock to cross the screen vertically.
#define RDX 2.5  // range for rock dx values (+/-)
#define RDY 2.5  // range for rock dy values (+/-)

float rnd(void);

#define crnd() (2*(rnd()-0.5))


int
init_rocks(void)
{
	int i;
	char a[MAX_PATH_LEN];
	SDL_Surface *temp;

	for(i = 0; i<NROCKS; i++) {
		snprintf(a,MAX_PATH_LEN,add_path("sprites/rock%02d.png"),i);
		NULLERROR(temp = IMG_Load(a));
		NULLERROR(surf_rock[i] = SDL_DisplayFormat(temp));
		get_shape(surf_rock[i], &rock_shapes[i]);
	}
	return 0;
}

void
reset_rocks(void)
{
	int i;

	for(i = 0; i<MAXROCKS; i++) rock[i].active = 0;
	nrocks = I_ROCKS;
	nrocks_timer = 0;
}

enum { LEFT, RIGHT, TOP, BOTTOM };


// compute the number of rocks/second that should be coming from each side

// compute the speed ranges of rocks coming from each side
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
	return sqrt(min * min + rnd() * (max * max - min * min));
}

void
new_rocks(void)
{
	int i,j;
	float ti[4];
	float rmin[4];
	float rmax[4];

	if(nrocks < F_ROCKS) {
		nrocks_timer += ticks_since_last;
		if(nrocks_timer >= nrocks_inc_ticks) {
			nrocks_timer -= nrocks_inc_ticks;
			nrocks++;
		}
	}

	rock_sides(ti, rmin, rmax);

	// loop through the four sides of the screen
	for(i=0; i<4; i++) {
		// see if we generate a rock for this side this frame
		rtimers[i] += ti[i]*gamerate/20;
		while(rtimers[i] >= 1) {
			rtimers[i] -= 1;
			j=0;
			while(rockptr->active && j<MAXROCKS) {
				if(++rockptr - rock >= MAXROCKS) rockptr = rock;
				j++;
			}
			if(!rockptr->active) {
				rockptr->type_number = random() % NROCKS;
				rockptr->image = surf_rock[rockptr->type_number];
				rockptr->shape = &rock_shapes[rockptr->type_number];
				switch(i) {
					case RIGHT:
						rockptr->x = XSIZE;
						rockptr->y = rnd()*(YSIZE + rockptr->image->h);

						rockptr->dx = -weighted_rnd_range(rmin[i], rmax[i]) + screendx;
						rockptr->dy = RDY*crnd();
						break;
					case LEFT:
						rockptr->x = -rockptr->image->w;
						rockptr->y = rnd()*(YSIZE + rockptr->image->h);

						rockptr->dx = weighted_rnd_range(rmin[i], rmax[i]) + screendx;
						rockptr->dy = RDY*crnd();
						break;
					case BOTTOM:
						rockptr->x = rnd()*(XSIZE + rockptr->image->w);
						rockptr->y = YSIZE;

						rockptr->dx = RDX*crnd();
						rockptr->dy = -weighted_rnd_range(rmin[i], rmax[i]) + screendy;
						break;
					case TOP:
						rockptr->x = rnd()*(XSIZE + rockptr->image->w);
						rockptr->y = -rockptr->image->h;

						rockptr->dx = RDX*crnd();
						rockptr->dy = weighted_rnd_range(rmin[i], rmax[i]) + screendy;
						break;
				}

				rockptr->active = 1;
			}
		}
	}
}

void
move_rocks(void)
{
	int i;

	// Move all the rocks
	for(i = 0; i < MAXROCKS; i++) {
		if(rock[i].active) {
			// move
			rock[i].x += (rock[i].dx-screendx)*gamerate;
			rock[i].y += (rock[i].dy-screendy)*gamerate;
			// clip
			if(rock[i].x < -rock[i].image->w || rock[i].x >= XSIZE
					|| rock[i].y < -rock[i].image->h || rock[i].y >= YSIZE) {
				rock[i].active = 0;
			}
		}
	}
}

void
draw_rocks(void)
{
	int i;
	SDL_Rect src, dest;

	src.x = 0; src.y = 0;

	for(i = 0; i<MAXROCKS; i++) {
		if(rock[i].active) {
			src.w = rock[i].image->w;
			src.h = rock[i].image->h;

			dest.w = src.w;
			dest.h = src.h;
			dest.x = (int) rock[i].x;
			dest.y = (int) rock[i].y;

			SDL_BlitSurface(rock[i].image,&src,surf_screen,&dest);

		}
	}
}

int
hit_rocks(float x, float y, struct shape *shape)
{
	int i;

	for(i=0; i<MAXROCKS; i++) {
		if(rock[i].active) {
			if(collide(x-rock[i].x, y-rock[i].y, rock[i].shape, shape)) 
				return 1;
		}
	}
	return 0;
}

void
blast_rocks(float x, float y, float radius, int onlyslow)
{
	int i;
	float dx, dy, n;

	if(onlyslow) return;

	for(i = 0; i<MAXROCKS; i++ ) {
		if(rock[i].x <= 0) continue;

		// This makes it so your explosion from dying magically doesn't leave
		// any rocks that aren't moving much on the x axis. If onlyslow is set,
		// only rocks that are barely moving will be pushed.
		if(onlyslow && (rock[i].dx-screendx < -4 || rock[i].dx-screendx > 3)) continue;

		dx = rock[i].x - x;
		dy = rock[i].y - y;

		n = sqrt(dx*dx + dy*dy);
		if(n < radius) {
			n *= 15;
			rock[i].dx += 54.0*dx/n;
			rock[i].dy += 54.0*dy/n;
		}
	}
}
