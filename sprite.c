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

	if(SDL_MUSTLOCK(s->image)) { SDL_LockSurface(s->image); }
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
	if(SDL_MUSTLOCK(s->image)) { SDL_UnlockSurface(s->image); }
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
	gw = (XSIZE + 2*grid_size) / grid_size; // -grid-size to XSIZE inclusive (so sprites can be just off either edge)
	gh = (YSIZE + 2*grid_size) / grid_size;

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
	if(b >= gw*gh || b < 0) {
		fprintf(stderr, "square(%i, %i, %i) = %i\n", x, y, set, b);
		((int*)0)[0] = 0;
	}
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


// xov: number of bits of overlap
// bit: number of bits in from the left edge of amask where bmask is
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

// xov: number of bits/pixels of horizontal overlap
// yov: number of bits/pixels of vertical overlap
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
		amask = a->mask + word;
		bmask = b->mask + ((b->h - yov) * b->mask_w);
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

/*
 * BOUNCE THEORY
 *
 * ******************  In 1 Dimension  *****************
 *
 * For now we will imagine bouncing A and B off each other in 1 dimension (along
 * a line). We can safely save the other dimension for later.
 *
 * A and B are the same weight, and are both traveling 1m/sec, to collide right
 * at the origin. With perfect bounciness, their full momentum is reversed.
 *
 * If we cut the weight of A down by half, then the center of our colision will
 * drift towards A (the speeds of A and B are not simply reversed as in our last
 * example.) However, there is always a place between A and B on the line (I'll
 * call it x) such that the speeds of A and B relative to x, are simply
 * reversed. Thus we can find the new speed for A like so:
 *
 *     new A = x -(A - x)
 *
 *     new B = x -(B - x)
 * 
 * or, simply:
 *
 *     new A = 2x - A
 *
 *     new B = 2x - B
 *
 *
 * this point x is the sort of center of momentum. If, instead of bouncing, A
 * and B just globbed together, x would be center of the new glob.
 *
 * x is the point where there's an equal amount of force coming in from both
 * sides. ie the weighted average of the speeds of A and B.
 *
 * average force = (A force + B force) / total mass
 *
 * x.speed = (a.speed * a.mass + b.speed * b.mass) / (a.mass + b.mas)
 *
 * then we apply the formula above for calculating the new A and B.
 *
 *
 *
 *
 * ******************  In 2 Dimensions  *****************
 *
 * OK, that's how we do it in 1D. Now we need to deal with 2D.
 * 
 * Imagine (or draw) the two balls just as they are bouncing off each other.
 * Imagine drawing a line through the centers of the balls. The balls are
 * exerting force on each other only along this axis. So if we rotate
 * everything, we can do our earlier 1D math along this line.
 *
 * It doesn't matter what direction the balls are going in, they only exert
 * force on each other along this line. What we will do is to compute the part
 * of the balls' momentum that is going along this line, and bounce it according
 * to our math above. The other part is unaffected by the bounce, and we can
 * just leave it alone.
 *
 * To get this component of the balls' momentum, we can use the dot product.
 *
 *     dot(U, V) = length(U) * length(V) * cos(angle between U and V)
 *
 * If U is a length 1 vector, then dot(U, V) is the length of the component of V
 * in the direction of U.  So the components of V are:
 *
 *     U * dot(U, V)      parallel to U
 *
 *     V - U * dot(U, V)  perpendicular to U
 *
 * To do the actual bounce, we compute the unit vector between the center of the
 * two balls, compute the components of the balls' speeds along this vector (A
 * and B), and then bounce them according to the math above:
 *
 *     new A = 2x - A
 *
 *     new B = 2x - B
 *
 * But we rewrite it in relative terms:
 *
 *     new A = A + 2(x-A)
 *
 *     new B = B + 2(x-B)
 */

void
bounce(Sprite *a, Sprite *b)
{
	float x, y, n;  // (x, y) is unit vector from a to b.
	float va, vb;   // va, vb are balls' speeds along (x, y)
	float ma, mb;   // ma, mb are the balls' masses.
	float vc;       // vc is the "center of momentum"

	// (x, y) is unit vector pointing from A's center to B's center.
	x = (b->x + b->w / 2) - (a->x + a->w / 2);
	y = (b->y + b->h / 2) - (a->y + a->h / 2);
	n = sqrt(x*x + y*y); x /= n; y /= n;

	// velocities along (x, y)
	va = x*a->dx + y*a->dy;
	vb = x*b->dx + y*b->dy;
	if(vb-va > 0) return;  // don't bounce if we're already moving away.

	// get masses and compute "center" speed
	ma = sprite_mass(a); mb = sprite_mass(b);
	vc = (va*ma + vb*mb) / (ma+mb);

	// bounce off the center speed.
	a->dx += 2*x*(vc-va); a->dy += 2*y*(vc-va);
	b->dx += 2*x*(vc-vb); b->dy += 2*y*(vc-vb);
}
