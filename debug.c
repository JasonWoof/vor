#include "debug.h"

#include <stdio.h>

void
printf_surface(SDL_Surface *s, char *name)
{
	printf("SDL_Surface *%s = {\n", name);
		printf("\tflags = 0x%x;\n", s->flags);
		printf("\tformat = {\n");
			printf("\t\tBitsPerPixel = %d;\n", s->format->BitsPerPixel);
			printf("\t\tBytesPerPixel = %d;\n", s->format->BytesPerPixel);
			printf("\t\tmasks = 0x%x, 0x%x, 0x%x, 0x%x;\n", s->format->Rmask, s->format->Gmask,
					s->format->Bmask, s->format->Amask);
			printf("\t\tshifts = %d, %d, %d, %d;\n", s->format->Rshift, s->format->Gshift,
					s->format->Bshift, s->format->Ashift);
			printf("\t\tlosses = %d, %d, %d, %d;\n", s->format->Rloss, s->format->Gloss,
					s->format->Bloss, s->format->Aloss);
			printf("\t\tcolorkey = %d;\n", s->format->colorkey);
			printf("\t\talpha = %d;\n", s->format->alpha);
		printf("\t};\n");
		printf("\tw, h = %d, %d;\n", s->w, s->h);
		printf("\tpitch = %d;\n", s->pitch);
	printf("};\n");
}
