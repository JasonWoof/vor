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
int nrocks;

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
	nrocks = 41;
	return 0;
}

void
reset_rocks(void)
{
	int i;

	for(i = 0; i<MAXROCKS; i++) rock[i].active = 0;
}

enum { LEFT, RIGHT, TOP, BOTTOM };

void
rock_timer_increments(float *ti)
{
	float dx0,dx1, dy0,dy1;
	float hfactor, vfactor;
	int i;

	for(i=0; i<4; i++) ti[i] = 0;
	hfactor = nrocks/KH; vfactor = nrocks/KV;

	dx0 = -RDX - screendx; dx1 = RDX - screendx;
	dy0 = -RDY - screendy; dy1 = RDY - screendy;

	if(dx0 != 0) {
		if(dx0 < 0) {
			if(dx1 < 0) ti[RIGHT] = -(dx0+dx1)/2;
			else {
				ti[RIGHT] = -dx0/2;
				ti[LEFT] = dx1/2;
			}
		} else ti[LEFT] = (dx0+dx1)/2;
	}
	ti[LEFT] *= hfactor;
	ti[RIGHT] *= hfactor;

	if(dy0 != 0) {
		if(dy0 < 0) {
			if(dy1 < 0) ti[BOTTOM] = -(dy0+dy1)/2;
			else {
				ti[BOTTOM] = -dy0/2;
				ti[TOP] = dy1/2;
			}
		} else ti[TOP] = (dy0+dy1)/2;
	}
	ti[TOP] *= vfactor;
	ti[BOTTOM] *= vfactor;
}

void
new_rocks(void)
{
	int i,j;
	float ti[4];

	rock_timer_increments(ti);

	for(i=0; i<4; i++) {
		rtimers[i] += ti[i]*gamerate/20;
		if(rtimers[i] >= 1) {
			j=0;
			while(rockptr->active && j<MAXROCKS) {
				if(++rockptr - rock >= MAXROCKS) rockptr = rock;
				j++;
			}
			if(!rockptr->active) {
				rtimers[i] -= 1;
				rockptr->type_number = random() % NROCKS;
				rockptr->image = surf_rock[rockptr->type_number];
				rockptr->shape = &rock_shapes[rockptr->type_number];
				switch(i) {
					case RIGHT:
						rockptr->x = XSIZE;
						rockptr->y = rnd()*(YSIZE + rockptr->image->h);
						break;
					case LEFT:
						rockptr->x = -rockptr->image->w;
						rockptr->y = rnd()*(YSIZE + rockptr->image->h);
						break;
					case BOTTOM:
						rockptr->x = rnd()*(XSIZE + rockptr->image->w);
						rockptr->y = YSIZE;
						break;
					case TOP:
						rockptr->x = rnd()*(XSIZE + rockptr->image->w);
						rockptr->y = -rockptr->image->h;
						break;
				}

				rockptr->dx = RDX*crnd();
				rockptr->dy = RDY*crnd();

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
			n *= 20;
			rock[i].dx += 54.0*dx/n;
			rock[i].dy += 54.0*dy/n;
		}
	}
}
