#include <stdlib.h>
#include "common.h"
#include "sprite.h"

void
load_sprite(Sprite *s, char *filename)
{
	s->image = load_image(filename);
	if(s->image) get_shape(s);
}

void
get_shape(Sprite *s)
{
	int x, y;
	uint16_t *px, transp;
	uint32_t bits = 0, bit, *p;

	if(s->image->format->BytesPerPixel != 2) {
		fprintf(stderr, "get_shape(): not a 16-bit image!\n");
		exit(1);
	}

	s->w = s->image->w; s->h = s->image->h;
	s->mask_w = ((s->image->w+31)>>5);
	s->mask = malloc(4*s->mask_w*s->h);
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
			if(*px++ != transp) { bits |= bit; }
			bit >>= 1;
			if(!bit || x == s->image->w - 1) { *(p++) = bits; }
		}
		px = (uint16_t *) ((uint8_t *) px + s->image->pitch - 2*s->image->w);
	}
	SDL_UnlockSurface(s->image);
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

	if(b->x < a->x) { Sprite *tmp = a; a = b; b = tmp; }

	dx = b->x - a->x;
	dy = b->y - a->y;

	xov = max(min(a->w - dx, b->w), 0);

	if(dy >= 0) yov = max(min(a->h - dy, b->h), 0);
	else yov = -max(min(a->h - -dy, b->h), 0);

	if(xov == 0 || yov == 0) return false;
	else return mask_collide(xov, yov, a, b);
}

int
pixel_collide(Sprite *s, int x, int y)
{
	uint32_t pmask;
	
	if(x < s->x || y < s->y || x >= s->x + s->w || y >= s->y + s->h) return 0;

	x -= s->x; y -= s->y;
	pmask = 0x80000000 >> (x&0x1f);
	return s->mask[(y*s->mask_w) + (x>>5)] & pmask;
}
