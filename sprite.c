#include <stdlib.h>
#include "common.h"
#include "sprite.h"

void
load_sprite(Sprite *sprite, char *filename)
{
	struct base_sprite *spr = &sprite->sprite;
	spr->image = load_image(filename);
	if(!spr->image) return;
	if(!spr->shape) {
		spr->shape = malloc(sizeof(struct shape));
		if(!spr->shape) {
			fprintf(stderr, "load_sprite(): can't allocate shape structure.\n");
			exit(1);
		}
		get_shape(spr->image, spr->shape);
	}
}

void
get_shape(SDL_Surface *img, struct shape *s)
{
	int x, y;
	uint16_t *px, transp;
	uint32_t bits = 0, bit, *p;

	if(img->format->BytesPerPixel != 2) {
		fprintf(stderr, "get_shape(): not a 16-bit image!\n");
		exit(1);
	}

	s->area = 0;
	s->w = img->w; s->h = img->h;
	s->mw = ((img->w+31)>>5);
	s->mask = malloc(4*s->mw*s->h);
	if(!s->mask) {
		fprintf(stderr, "get_shape(): can't allocate bitmask.\n");
		exit(1);
	}

	SDL_LockSurface(img);
	px = img->pixels;
	transp = img->format->colorkey;
	p = s->mask;
	for(y=0; y<img->h; y++) {
		bit = 0;
		for(x=0; x<img->w; x++) {
			if(!bit) { bits = 0; bit = 0x80000000; }
			if(*px++ != transp) { bits |= bit; s->area++; }
			bit >>= 1;
			if(!bit || x == img->w - 1) { *(p++) = bits; }
		}
		px = (uint16_t *) ((uint8_t *) px + img->pitch - 2*img->w);
	}
	SDL_UnlockSurface(img);
}

static int
line_collide(int xov, struct shape *r, uint32_t *rbits, struct shape *s, uint32_t *sbits)
{
	int lshift, n, i, ret = 0;
	uint32_t lbits;
	struct shape *st;
	uint32_t *bt;


	if(xov < 0) {
		st = r; r = s; s = st;
		bt = rbits; rbits = sbits; sbits = bt;
		xov = -xov;
	}


	lshift = (r->w - xov) & 31; 
	rbits += (r->w - xov) >> 5;
	n = (xov + 31) >> 5;
	for(i=0; i<n-1; i++) {
		lbits = *rbits++ << lshift;
		lbits |= *rbits >> (32 - lshift);
		if(lbits & *sbits++) ret = 1;
	}
	lbits = *rbits << lshift;
	if(lbits & *sbits) ret = 1;

	return ret;
}

static int
mask_collide(int xov, int yov, struct shape *r, struct shape *s)
{
	int y, ry, sy;
	uint32_t *rbits, *sbits;

	if(yov > 0) {
		ry = r->h - yov; sy = 0;
		rbits = r->mask + (r->h - yov) * r->mw;
		sbits = s->mask;
	} else {
		ry = 0; sy = s->h + yov;
		rbits = r->mask;
		sbits = s->mask + (s->h + yov) * s->mw;
	}

	for(y=0; y<abs(yov); y++) {
		if(line_collide(xov, r, rbits, s, sbits)) return 1;
		rbits += r->mw; sbits += s->mw;
	}

	return 0;
}

int
collide(Sprite *r, Sprite *s)
{
	struct shape *rs = r->sprite.shape;
	struct shape *ss = s->sprite.shape;
	int dx = s->sprite.x - r->sprite.x;
	int dy = s->sprite.y - r->sprite.y;
	int xov, yov;

	if(dx >= 0) xov = max(min(rs->w - dx, ss->w), 0);
	else xov = -max(min(ss->w + dx, rs->w), 0);

	if(dy >= 0) yov = max(min(rs->h - dy, ss->h), 0);
	else yov = -max(min(ss->h + dy, rs->h), 0);

	if(xov == 0 || yov == 0) return false;
	else return mask_collide(xov, yov, rs, ss);
}

int
pixel_collide(unsigned int xoff, unsigned int yoff, struct shape *r)
{
	uint32_t pmask;
	
	if(xoff >= r->w || yoff >= r->h) return 0;

	pmask = 0x80000000 >> (xoff&0x1f);
	return r->mask[(yoff*r->mw) + (xoff>>5)] & pmask;
}
