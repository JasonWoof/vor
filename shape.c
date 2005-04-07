#include <stdlib.h>
#include "shape.h"

void
get_shape(SDL_Surface *img, struct shape *s)
{
	int x, y;
	uint16_t *px, transp;
	uint32_t bits;

	if(img->format->BytesPerPixel != 2) {
		fprintf(stderr, "get_shape(): not a 16-bit image!\n");
		exit(1);
	}

	s->w = img->w; s->h = img->h;
	s->mw = ((img->w+31)>>5) * img->h;
	s->mask = malloc(4*s->mw);
	if(!s->mask) {
		fprintf(stderr, "can't malloc bitmask");
		exit(1);
	}

	SDL_LockSurface(img);
	px = img->pixels;
	transp = img->format->colorkey;
	bits = 0;
	for(y=0; y<img->h; y++) {
		for(x=0; x<img->w; x++) {
			if(*px++ != transp) bits |= 1;
			if(x == img->w-1 || !(x+1)%32) {
				*(s->mask++) = bits;
				bits = 0;
			} else bits = bits << 1;
		}
		px = (uint16_t *) ((uint8_t *) px + img->pitch - 2*img->w);
	}
	SDL_UnlockSurface(img);
}

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

int
collide(int xdiff, int ydiff, struct shape *r, struct shape *s)
{
	int xov, yov;

	if(xdiff >= 0) xov = max(min(r->w-xdiff, s->w), 0);
	else xov = min(-min(s->w+xdiff, r->w), 0);

	if(ydiff >= 0) yov = max(min(r->h-ydiff, s->h), 0);
	else yov = min(-min(s->h+ydiff, r->h), 0);

	if(xov == 0 || yov == 0) return 0;  // bboxes hit?
	else return 1;
}
