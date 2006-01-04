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
#include "shape.h"

SDL_Surface *load_image(char *filename);

struct rock_struct {
	struct rock_struct *next;
	float x,y,dx,dy;
	SDL_Surface *image;
	struct shape *shape;
	int type_number;
}; 

struct rock_struct rocks[MAXROCKS], *free_rocks;

struct rock_struct **rock_buckets[2];
int n_buckets;
// we have two sets of buckets -- this variable tells which we are using.
int p;
int bw, bh;
int grid_size;

SDL_Surface *surf_rock[NROCKS];
struct shape rock_shapes[NROCKS];

// timers for rock generation.
float rtimers[4];

uint32_t nrocks;
float nrocks_timer;
float nrocks_inc_ticks = 2*60*20/(F_ROCKS-I_ROCKS);

// constants for rock generation.
#define KH (32*20)  // 32 s for a speed=1 rock to cross the screen horizontally.
#define KV (24*20)  // 24 s for a speed=1 rock to cross the screen vertically.
#define RDX 2.5  // range for rock dx values (+/-)
#define RDY 2.5  // range for rock dy values (+/-)

static inline struct rock_struct **
bucket(int x, int y, int p)
{
	int b = (1+x/grid_size) + bw*(1+y/grid_size);
	return &rock_buckets[p][b];
}

void
init_buckets(void)
{
	bw = (XSIZE+2*grid_size-1) / grid_size;
	bh = (YSIZE+2*grid_size-1) / grid_size;
	n_buckets = bw * bh;
	
	rock_buckets[0] = malloc(n_buckets * sizeof(struct rock_struct *));
	rock_buckets[1] = malloc(n_buckets * sizeof(struct rock_struct *));
	if(!rock_buckets[0] || !rock_buckets[1]) {
		fprintf(stderr, "Can't allocate rock buckets.\n");
		exit(1);
	}
	p = 0;
}

void
transfer_rock(struct rock_struct *r, struct rock_struct **from, struct rock_struct **to)
{
	*from = r->next;
	r->next = *to;
	*to = r;
}

void
reset_rocks(void)
{
	int i;

	for(i=0; i<MAXROCKS; i++) rocks[i].image = NULL;
	rocks[0].next = NULL; free_rocks = &rocks[MAXROCKS-1];
	for(i = 1; i<MAXROCKS; i++) rocks[i].next = &rocks[i-1];
	for(i = 0; i<n_buckets; i++) {
		rock_buckets[0][i] = NULL;
		rock_buckets[1][i] = NULL;
	}

	nrocks = I_ROCKS;
	nrocks_timer = 0;
}

#define ROCK_LEN sizeof("sprites/rockXX.png")

int
init_rocks(void)
{
	int i;
	char a[ROCK_LEN];
	int maxw=0, maxh=0;

	for(i = 0; i<NROCKS; i++) {
		snprintf(a, ROCK_LEN, "sprites/rock%02d.png", i);
		NULLERROR(surf_rock[i] = load_image(a));
		get_shape(surf_rock[i], &rock_shapes[i]);
		maxw = max(maxw, rock_shapes[i].w);
		maxh = max(maxh, rock_shapes[i].h);
	}
	grid_size = max(maxw, maxh) * 3 / 2;
	init_buckets();
	reset_rocks();
	return 0;
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
	int i;
	struct rock_struct *r;
	float ti[4];
	float rmin[4];
	float rmax[4];

	if(nrocks < F_ROCKS) {
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
			if(!free_rocks) return;  // sorry, we ran out of rocks!
			r = free_rocks;
			r->type_number = urnd() % NROCKS;
			r->image = surf_rock[r->type_number];
			r->shape = &rock_shapes[r->type_number];
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
					r->x = frnd()*(XSIZE + r->image->w);
					r->y = YSIZE;

					r->dx = RDX*crnd();
					r->dy = -weighted_rnd_range(rmin[i], rmax[i]) + screendy;
					break;
				case TOP:
					r->x = frnd()*(XSIZE + r->image->w);
					r->y = -r->image->h;

					r->dx = RDX*crnd();
					r->dy = weighted_rnd_range(rmin[i], rmax[i]) + screendy;
					break;
			}
			transfer_rock(r, &free_rocks, bucket(r->x, r->y, p));
		}
	}
}

void
move_rocks(void)
{
	int b;
	struct rock_struct **head;
	struct rock_struct *r;

	// Move all the rocks
	for(b=0; b<n_buckets; b++) {
		head=&rock_buckets[p][b]; r=*head;
		while(*head) {
			r=*head;

			// move
			r->x += (r->dx - screendx)*t_frame;
			r->y += (r->dy - screendy)*t_frame;

			// clip or resort into other bucket set
			// (either way we move it out of this list).
			if(r->x + r->image->w < 0 || r->x >= XSIZE
					|| r->y + r->image->h < 0 || r->y >= YSIZE) {
				transfer_rock(r, head, &free_rocks);
				r->image = NULL;
			} else transfer_rock(r, head, bucket(r->x, r->y, 1-p));
		}
	}
	p = 1-p;  // switch current set of buckets.
}

void
draw_rocks(void)
{
	int i;
	SDL_Rect dest;

	for(i=0; i<MAXROCKS; i++) {
		if(!rocks[i].image) continue;
		dest.x = rocks[i].x; dest.y = rocks[i].y;
		SDL_BlitSurface(rocks[i].image,NULL,surf_screen,&dest);
	}
}

int
hit_in_bucket(struct rock_struct *r, float x, float y, struct shape *shape)
{
	for(; r; r=r->next) {
		if(collide(x - r->x, y - r->y, r->shape, shape)) return 1;
	}
	return 0;
}

int
hit_rocks(float x, float y, struct shape *shape)
{
	struct rock_struct **b = bucket(x, y, p);
	int bdx = ((int)x+shape->w)/grid_size - (int)x/grid_size;
	int bdy = ((int)y+shape->h)/grid_size - (int)y/grid_size;
	if(hit_in_bucket(*b, x, y, shape)) return 1;
	if(hit_in_bucket(*(b-1), x, y, shape)) return 1;
	if(hit_in_bucket(*(b-bw), x, y, shape)) return 1;
	if(hit_in_bucket(*(b-bw-1), x, y, shape)) return 1;

	if(bdx) {
		if(hit_in_bucket(*(b+1), x, y, shape)) return 1;
		if(hit_in_bucket(*(b+1-bw), x, y, shape)) return 1;
	}
	if(bdy) {
		if(hit_in_bucket(*(b+bw), x, y, shape)) return 1;
		if(hit_in_bucket(*(b+bw-1), x, y, shape)) return 1;
	}
	if(bdx && bdy && hit_in_bucket(*(b+bw+1), x, y, shape)) return 1;
	return 0;
}

int
pixel_hit_in_bucket(struct rock_struct *r, float x, float y)
{
	for(; r; r=r->next) {
		if(x < r->x || y < r->y) continue;
		if(pixel_collide(x - r->x, y - r->y, r->shape)) return 1;
	}
	return 0;
}

int
pixel_hit_rocks(float x, float y)
{
	struct rock_struct **b = bucket(x, y, p);
	if(pixel_hit_in_bucket(*b, x, y)) return 1;
	if(pixel_hit_in_bucket(*(b-1), x, y)) return 1;
	if(pixel_hit_in_bucket(*(b-bw), x, y)) return 1;
	if(pixel_hit_in_bucket(*(b-bw-1), x, y)) return 1;
	return 0;
}

void
blast_rocks(float x, float y, float radius, int onlyslow)
{
	int b;
	struct rock_struct *r;
	float dx, dy, n;

	if(onlyslow) return;

	for(b=0; b<n_buckets; b++) {
		for(r=rock_buckets[p][b]; r; r=r->next) {
			if(r->x <= 0) continue;

			// This makes it so your explosion from dying magically doesn't leave
			// any rocks that aren't moving much on the x axis. If onlyslow is set,
			// only rocks that are barely moving will be pushed.
			if(onlyslow && (r->dx - screendx < -4 || r->dx - screendx > 3)) continue;

			dx = r->x - x;
			dy = r->y - y;

			n = sqrt(dx*dx + dy*dy);
			if(n < radius) {
				n *= 15;
				r->dx += 54.0*dx/n;
				r->dy += 54.0*dy/n;
			}
		}
	}
}
