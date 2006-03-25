#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "common.h"
#include "globals.h"
#include "sprite.h"
#include "rocks.h"

SDL_Surface *load_image(char *filename);
void load_ship(void);

// 2 sets of sprites, sorted by position
static Sprite **sprites[2] = { NULL, NULL };

// which set are we using?
static int set = 0;

// size of squares into which sprites are sorted.
static int grid_size = 0;

// screen size in grid squares.
static int gw = 0, gh = 0;

// lists of free sprites, by type.
Sprite *free_sprites[N_TYPES];


static void
get_shape(Sprite *s)
{
	int x, y;
	uint16_t *px, transp;
	uint32_t bits = 0, bit, *p;

	s->area = 0;
	if(s->image->format->BytesPerPixel != 2) {
		fprintf(stderr, "get_shape(): not a 16-bit image!\n");
		exit(1);
	}

	s->w = s->image->w; s->h = s->image->h;
	grid_size = max(grid_size, max(s->w, s->h));
	s->mask_w = ((s->w+31)>>5);
	s->mask = malloc(s->mask_w*s->h*sizeof(uint32_t));
	if(!s->mask) {
		fprintf(stderr, "get_shape(): can't allocate bitmask.\n");
		exit(1);
	}

	SDL_LockSurface(s->image);
	px = s->image->pixels;
	transp = s->image->format->colorkey;
	p = s->mask;
	for(y=0; y<s->image->h; y++) {
		bit = 0;
		for(x=0; x<s->image->w; x++) {
			if(!bit) { bits = 0; bit = 0x80000000; }
			if(*px++ != transp) { bits |= bit; s->area++; }
			bit >>= 1;
			if(!bit || x == s->image->w - 1) { *(p++) = bits; }
		}
		px = (uint16_t *) ((uint8_t *) px + s->image->pitch - 2*s->image->w);
	}
	SDL_UnlockSurface(s->image);
}


void
load_sprite(Sprite *s, char *filename)
{
	s->image = load_image(filename);
	if(s->image) get_shape(s);
}


static void
load_sprites(void)
{
	load_ship();
	load_rocks();
}


void
init_sprites(void)
{
	load_sprites();

	grid_size = grid_size * 3 / 2;
	gw = (XSIZE-1 + 2*grid_size) / grid_size;
	gh = (YSIZE-1 + 2*grid_size) / grid_size;

	sprites[0] = malloc(2 * gw * gh * sizeof(Sprite *));
	sprites[1] = (void *)sprites[0] + gw * gh * sizeof(Sprite *);
	if(!sprites[0]) {
		fprintf(stderr, "init_sprites(): can't allocate grid squares.\n");
		exit(1);
	}
	memset(sprites[0], 0, 2 * gw * gh * sizeof(Sprite *));
	set = 0;
}

static inline Sprite **
square(int x, int y, int set)
{
	int b = (x+grid_size)/grid_size + gw*((y+grid_size)/grid_size);
	return &sprites[set][b];
}

void
add_sprite(Sprite *s)
{
	insert_sprite(square(s->x, s->y, set), s);
}

void
reset_sprites(void)
{
	int i;

	for(i=0; i<gw*gh; i++)
		while(sprites[set][i]) {
			Sprite *s = remove_sprite(&sprites[set][i]);
			insert_sprite(&free_sprites[s->type], s);
			s->flags = 0;
		}
}

void
move_sprite(Sprite *s)
{
	if(s->flags & MOVE) {
		s->x += (s->dx - screendx)*t_frame;
		s->y += (s->dy - screendy)*t_frame;
	}
}

void
sort_sprite(Sprite *s)
{
	// clip it, or sort it into the other set of sprites.
	if(s->x + s->w < 0 || s->x >= XSIZE
	   || s->y + s->h < 0 || s->y >= YSIZE) {
		insert_sprite(&free_sprites[s->type], s);
		s->flags = 0;
	} else insert_sprite(square(s->x, s->y, 1-set), s);
}

void
move_sprites(void)
{
	int sq;
	Sprite **head;

	// Move all the sprites
	for(sq=0; sq<gw*gh; sq++) {
		head=&sprites[set][sq];
		while(*head) {
			Sprite *s = remove_sprite(head);
			move_sprite(s); sort_sprite(s);
		}
	}
	set = 1-set;  // switch to other set of sprites.
}


static int
line_collide(int xov, unsigned bit, uint32_t *amask, uint32_t *bmask)
{
	int i, words = (xov-1) >> 5;
	uint32_t abits;

	for(i=0; i<words; i++) {
		abits = *amask++ << bit;
		abits |= *amask >> (32-bit);
		if(abits & *bmask++) return true;
	}
	abits = *amask << bit;
	if(abits & *bmask) return true;

	return false;
}

static int
mask_collide(int xov, int yov, Sprite *a, Sprite *b)
{
	int y;
	int xoffset = a->w - xov;
	int word = xoffset >> 5, bit = xoffset & 31;
	uint32_t *amask = a->mask, *bmask = b->mask;

	if(yov > 0) {
		amask = a->mask + ((a->h - yov) * a->mask_w) + word;
		bmask = b->mask;
	} else {
		yov = -yov;
		amask = a->mask;
		bmask = b->mask + ((b->h - yov) * b->mask_w) + word;
	}

	for(y=0; y<yov; y++) {
		if(line_collide(xov, bit, amask, bmask)) return 1;
		amask += a->mask_w; bmask += b->mask_w;
	}

	return 0;
}

int
collide(Sprite *a, Sprite *b)
{
	int dx, dy, xov, yov;

	if(!COLLIDES(a) || !COLLIDES(b)) return false;

	if(b->x < a->x) { Sprite *tmp = a; a = b; b = tmp; }

	dx = b->x - a->x;
	dy = b->y - a->y;

	xov = max(min(a->w - dx, b->w), 0);

	if(dy >= 0) yov = max(min(a->h - dy, b->h), 0);
	else yov = -max(min(b->h - -dy, a->h), 0);

	if(xov == 0 || yov == 0) return false;
	else return mask_collide(xov, yov, a, b);
}

void
collide_with_list(Sprite *s, Sprite *list)
{
	for(; list; list=list->next)
		if(collide(s, list)) do_collision(s, list);
}

void
collisions(void)
{
	int i, end = gw*gh;
	Sprite *s;
	for(i=0; i<end; i++) {
		for(s=sprites[set][i]; s; s=s->next) {
			collide_with_list(s, s->next);
			if(i+1 < end) collide_with_list(s, sprites[set][i+1]);
			if(i+gw < end) collide_with_list(s, sprites[set][i+gw]);
			if(i+gw+1 < end) collide_with_list(s, sprites[set][i+gw+1]);
		}
	}
}

Sprite *
hit_in_square(Sprite *r, Sprite *s)
{
	for(; r; r=r->next)
		if(collide(r, s)) break;
	return r;
}

Sprite *
collides(Sprite *s)
{
	int l, r, t, b;
	Sprite **sq;
	Sprite *c;

	l = (s->x + grid_size) / grid_size;
	r = (s->x + s->w + grid_size) / grid_size;
	t = (s->y + grid_size) / grid_size;
	b = (s->y + s->h + grid_size) / grid_size;
	sq = &sprites[set][l + t*gw];

	if((c = hit_in_square(*sq, s))) return c;
	if(l > 0 && (c = hit_in_square(*(sq-1), s))) return c;
	if(t > 0 && (c = hit_in_square(*(sq-gw), s))) return c;
	if(l > 0 && t > 0 && (c = hit_in_square(*(sq-1-gw), s))) return c;

	if(r > l) {
		if((c = hit_in_square(*(sq+1), s))) return c;
		if(t > 0 && hit_in_square(*(sq+1-gw), s)) return c;
	}
	if(b > t) {
		if((c = hit_in_square(*(sq+gw), s))) return c;
		if(l > 0 && (c = hit_in_square(*(sq-1+gw), s))) return c;
	}
	if(r > l && b > t && (c = hit_in_square(*(sq+1+gw), s))) return c;
	return NULL;
}

int
pixel_collide(Sprite *s, int x, int y)
{
	uint32_t pmask;

	if(!COLLIDES(s)) return false;
	
	if(x < s->x || y < s->y || x >= s->x + s->w || y >= s->y + s->h) return 0;

	x -= s->x; y -= s->y;
	pmask = 0x80000000 >> (x&0x1f);
	return s->mask[(y*s->mask_w) + (x>>5)] & pmask;
}

Sprite *
pixel_hit_in_square(Sprite *r, float x, float y)
{
	for(; r; r=r->next) {
		if(COLLIDES(r) && pixel_collide(r, x, y)) return r;
	}
	return 0;
}

Sprite *
pixel_collides(float x, float y)
{
	int l, t;
	Sprite **sq;
	Sprite *ret;

	l = (x + grid_size) / grid_size; t = (y + grid_size) / grid_size;
	sq = &sprites[set][l + t*gw];
	if((ret = pixel_hit_in_square(*sq, x, y))) return ret;
	if(l > 0 && (ret = pixel_hit_in_square(*(sq-1), x, y))) return ret;
	if(t > 0 && (ret = pixel_hit_in_square(*(sq-gw), x, y))) return ret;
	if(l > 0 && t > 0 && (ret = pixel_hit_in_square(*(sq-1-gw), x, y))) return ret;
	return 0;
}


float
sprite_mass(Sprite *s)
{
	if(s->type == SHIP) return s->area;
	else if(s->type == ROCK) return 3 * s->area;
	else return 0;
}

void
bounce(Sprite *a, Sprite *b)
{
	float x, y, n;
	float va, vb;
	float ma, mb, mr;

	// (x, y) is unit vector pointing from A's center to B's center.
	x = (b->x + b->w / 2) - (a->x + a->w / 2);
	y = (b->y + b->h / 2) - (a->y + a->h / 2);
	n = sqrt(x*x + y*y); x /= n; y /= n;

	// velocities along (x, y), or 0 if already moving away.
	va = max(x*a->dx + y*a->dy, 0);
	vb = min(x*b->dx + y*b->dy, 0);

	// mass ratio
	ma = sprite_mass(a); mb = sprite_mass(b);
	if(ma && mb) mr = mb/ma; else mr = 1;

	a->dx += x*(mb*vb - ma*va)/ma; a->dy += y*(mb*vb - ma*va)/ma;
	b->dx += x*(ma*va - mb*vb)/mb; b->dy += y*(ma*va - mb*vb)/mb;
}
