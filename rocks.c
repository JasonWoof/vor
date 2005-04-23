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
	int dead;  // has been blown out of the way
	           // to make room for a new ship appearing.
	SDL_Surface *image;
	struct shape *shape;
	int type_number;
}; 

struct rock_struct rock[MAXROCKS], *rockptr = rock;

float rockrate,rockspeed;

SDL_Surface *surf_rock[NROCKS];
struct shape rock_shapes[NROCKS];

int countdown = 0;

float rnd(void);

uint32_t area;

int
init_rocks(void)
{
	int i;
	char a[MAX_PATH_LEN];
	SDL_Surface *temp;

	area = 0;

	for(i = 0; i<NROCKS; i++) {
		snprintf(a,MAX_PATH_LEN,add_path("sprites/rock%02d.png"),i);
		NULLERROR(temp = IMG_Load(a));
		NULLERROR(surf_rock[i] = SDL_DisplayFormat(temp));
		get_shape(surf_rock[i], &rock_shapes[i]);
	}
	return 0;
}

void
new_rocks(void)
{
	if(--countdown <= 0 && (rnd()*100.0<(rockrate += 0.025))) {
		// Possibly create a rock
		rockptr++;
		if(rockptr-rock >= MAXROCKS) {
			rockptr = rock;
		}
		if(!rockptr->active) {
			rockptr->dx = -(rockspeed)*(1 + rnd());
			rockptr->dy = rnd()-0.5;
			rockptr->type_number = random() % NROCKS;
			rockptr->image = surf_rock[rockptr->type_number];
			rockptr->shape = &rock_shapes[rockptr->type_number];
			rockptr->x = (float)XSIZE;
			rockptr->y = rnd()*(YSIZE + rockptr->image->h);
			rockptr->active = 1;
			area += rockptr->shape->area;
		}
		if(gamerate>0.1) {
			countdown = (int)(ROCKRATE/gamerate);
		} else {
			countdown = 0;
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
			rock[i].x += rock[i].dx*gamerate;
			rock[i].y += rock[i].dy*gamerate + yscroll;
			if(rock[i].y > YSIZE || rock[i].y < -rock[i].image->h) {
				if(rock[i].dead) {
					area -= rock[i].shape->area;
					rock[i].dead = 0;
					rock[i].active = 0;
				} else {
					// wrap
					rock[i].y = (YSIZE - rock[i].image->h) - rock[i].y;
					rock[i].y += (rock[i].dy*gamerate + yscroll) * 1.01;
				}
			}
			if(rock[i].x < -rock[i].image->w || rock[i].x > XSIZE) {
				area -= rock[i].shape->area;
				rock[i].active = 0;
				rock[i].dead = 0;
			}
		}
	}
}

void
reset_rocks(void)
{
	int i;

	area = 0;
	for(i = 0; i<MAXROCKS; i++ ) {
		rock[i].active = 0;
		rock[i].dead = 0;
	}

	rockrate = 54.0;
	rockspeed = 5.0;
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
		if(onlyslow && (!rock[i].dead || rock[i].dx < -4 || rock[i].dx > 3)) {
			continue;
		}

		dx = rock[i].x - x;
		dy = rock[i].y - y;

		n = sqrt(dx*dx + dy*dy);
		if(n < radius) {
			n *= 20;
			rock[i].dx += rockrate*(dx+30)/n;
			rock[i].dy += rockrate*dy/n;
			rock[i].dead = 1;
		}
	}
}
