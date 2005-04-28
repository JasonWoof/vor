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

float rockrate;

SDL_Surface *surf_rock[NROCKS];
struct shape rock_shapes[NROCKS];

float rockhtimer = 0;
int nrocks = 41;

float rnd(void);

// used for rock generation.
#define KH (1.0/32.0)
#define KV (1.0/24.0)
#define RXMIN 5.0
#define RXMAX 10.0
#define RYMIN (-0.5)
#define RYMAX 0.5


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

	for(i = 0; i<MAXROCKS; i++ ) rock[i].active = 0;

	rockrate = 54.0;
}

void
new_rocks(void)
{
	int i = 0;
	float hfactor = KH*nrocks*7.5;

	rockhtimer += hfactor*gamerate/20;
	if(rockhtimer >= 1) {
		while(rockptr->active && i<MAXROCKS) {
			if(++rockptr - rock >= MAXROCKS) rockptr = rock;
			i++;
		}
		if(!rockptr->active) {
			rockhtimer -= 1;
			rockptr->dx = -5.0*(1.0 + rnd());
			rockptr->dy = rnd()-0.5;
			rockptr->type_number = random() % NROCKS;
			rockptr->image = surf_rock[rockptr->type_number];
			rockptr->shape = &rock_shapes[rockptr->type_number];
			rockptr->x = (float)XSIZE;
			rockptr->y = rnd()*(YSIZE + rockptr->image->h);
			rockptr->active = 1;
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
			rock[i].x += rock[i].dx*gamerate;
			rock[i].y += rock[i].dy*gamerate + yscroll;
			// clip
			if(rock[i].y < -rock[i].image->h || rock[i].y > YSIZE) {
				// rock[i].active = 0;
				rock[i].y = (YSIZE - rock[i].image->h) - rock[i].y;
				rock[i].y += (rock[i].dy*gamerate + yscroll) * 1.01;
			}
			if(rock[i].x < -rock[i].image->w || rock[i].x > XSIZE) rock[i].active = 0;
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
		if(onlyslow && (rock[i].dx < -4 || rock[i].dx > 3)) continue;

		dx = rock[i].x - x;
		dy = rock[i].y - y;

		n = sqrt(dx*dx + dy*dy);
		if(n < radius) {
			n *= 20;
			rock[i].dx += rockrate*(dx+30)/n;
			rock[i].dy += rockrate*dy/n;
		}
	}
}
