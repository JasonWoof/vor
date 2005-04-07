#include <SDL/SDL.h>
#include <stdint.h>

struct shape {
	uint32_t w, h;
	uint32_t mw;
	uint32_t *mask;
};

void get_shape(SDL_Surface *img, struct shape *s);
int collide(int xdiff, int ydiff, struct shape *r, struct shape *s);
