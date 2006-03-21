#ifndef VOR_SHAPE_H
#define VOR_SHAPE_H

#include <SDL.h>
#include <inttypes.h>

typedef struct sprite Sprite;

#define SPRITE(x) ((Sprite *) (x))

#define BASE_SPRITE 0
#define SHIP_SPRITE 1
#define ROCK_SPRITE 2

struct sprite {
	uint8_t type;
	Sprite *next;
	float x, y;
	float dx, dy;
	SDL_Surface *image;
	int w, h;
	int mask_w;
	uint32_t *mask;
};

void get_shape(Sprite *s);
int collide(Sprite *r, Sprite *s);
int pixel_collide(Sprite *s, int x, int y);



// extended sprites

struct ship {
	// core sprite fields
	uint8_t sprite_type;
	struct ship *next;
	float x, y;
	float dx, dy;
	SDL_Surface *image;
	int w, h;
	int mask_w;
	uint32_t *mask;
	// SHIP extras
	int lives;
	int jets;
};

struct rock {
	// core sprite fields
	uint8_t sprite_type;
	struct rock *next;
	float x, y;
	float dx, dy;
	SDL_Surface *image;
	int w, h;
	int mask_w;
	uint32_t *mask;
	// ROCK extras
	int type;
};

SDL_Surface *load_image(char *filename);
void load_sprite(Sprite *sprite, char *filename);

#endif // VOR_SHAPE_H
