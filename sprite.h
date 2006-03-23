#ifndef VOR_SPRITE_H
#define VOR_SPRITE_H

#include <SDL.h>
#include <inttypes.h>

typedef struct sprite Sprite;

#define SPRITE(x) ((Sprite *) (x))

#define BASE_SPRITE 0
#define SHIP_SPRITE 1
#define ROCK_SPRITE 2
#define N_TYPES 3

struct sprite {
	int8_t type;
	Sprite *next;
	float x, y;
	float dx, dy;
	SDL_Surface *image;
	int w, h;
	int mask_w;
	uint32_t *mask;
};

Sprite *free_sprites[N_TYPES];  // lists of free sprites, by type.

void init_sprites(void);
void add_sprite(Sprite *s);
void move_sprite(Sprite *s);
void move_sprites(void);

Sprite *collides(Sprite *s);
int pixel_collides(float x, float y);
void load_sprite(Sprite *sprite, char *filename);

void bounce(Sprite *a, Sprite *b);


// extended sprites

struct ship {
	// core sprite fields
	int8_t sprite_type;
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
	int8_t sprite_type;
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



static inline void
insert_sprite(Sprite **head, Sprite *s)
{
	s->next = *head;
	*head = s;
}


static inline Sprite *
remove_sprite(Sprite **head)
{
	Sprite *s = *head;
	*head = s->next;
	return s;
}


static inline void
draw_sprite(Sprite *s)
{
	SDL_Rect dest;
	if(s->type == NONE) return;
	dest.x = s->x; dest.y = s->y;
	SDL_BlitSurface(s->image, NULL, surf_screen, &dest);
}

#endif // VOR_SPRITE_H
