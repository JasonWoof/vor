#ifndef VOR_SHAPE_H
#define VOR_SHAPE_H

#include <SDL/SDL.h>
#include <stdint.h>

struct shape {
	int w, h;
	int mw; // mask width (number of uint32's)
	uint32_t *mask;
	uint32_t area;
};

void get_shape(SDL_Surface *img, struct shape *s);
int collide(int xdiff, int ydiff, struct shape *r, struct shape *s);
int pixel_collide(unsigned int xoff, unsigned int yoff, struct shape *r);

#endif // VOR_SHAPE_H
