#ifndef VOR_SHAPE_H
#define VOR_SHAPE_H

#include <SDL/SDL.h>
#include <inttypes.h>


// Shape stuff

struct shape {
	int w, h;
	int mw; // mask width (number of uint32's)
	uint32_t *mask;
	uint32_t area;
};

void get_shape(SDL_Surface *img, struct shape *s);
int collide(int xdiff, int ydiff, struct shape *r, struct shape *s);
int pixel_collide(unsigned int xoff, unsigned int yoff, struct shape *r);



// Sprite stuff

typedef union sprite Sprite;

#define SPRITE(x) ((Sprite *) (x))

struct base_sprite {
	uint8_t type;
	Sprite *next;
	float x, y;
	float dx, dy;
	SDL_Surface *image;
	struct shape *shape;
};

struct rock {
	// core sprite fields
	uint8_t sprite_type;
	Sprite *next;
	float x, y;
	float dx, dy;
	SDL_Surface *image;
	struct shape *shape;
	// ROCK extras
	int type;
};

struct ship {
	// core sprite fields
	uint8_t sprite_type;
	Sprite *next;
	float x, y;
	float dx, dy;
	SDL_Surface *image;
	struct shape *shape;
	// SHIP extras
	int lives;
	int jets;
};

union sprite {
	uint8_t type;
	struct base_sprite sprite;
	struct rock rock;
	struct ship ship;
};

#define BASE_SPRITE 0
#define SHIP_SPRITE 1
#define ROCK_SPRITE 2

#endif // VOR_SHAPE_H
