#ifndef VOR_SHAPE_H
#define VOR_SHAPE_H

#include <SDL.h>
#include <inttypes.h>

typedef union sprite Sprite;


// Shape stuff

struct shape {
	int w, h;
	int mw; // mask width (number of uint32's)
	uint32_t *mask;
	uint32_t area;
};

void get_shape(SDL_Surface *img, struct shape *s);
int collide(Sprite *r, Sprite *s);
int pixel_collide(unsigned int xdiff, unsigned int ydiff, struct shape *r);



// Sprite stuff

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

SDL_Surface *load_image(char *filename);
void load_sprite(Sprite *sprite, char *filename);

#endif // VOR_SHAPE_H
