#include "debug.h"
#include "shape.h"

#include <stdio.h>
#include <stdint.h>

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

void
printb(uint32_t n, int bits)
{
	int i;

	for(i=0; i<bits; i++) {
		if(n & 0x80000000) putchar('1'); else putchar('0');
		n = n << 1;
	}
}

void
print_mask(struct shape *s)
{
	int i, j;

	for(i=0; i<s->h; i++) {
		for(j=0; j<s->mw-1; j++) printb(s->mask[s->mw*i+j], 32);
		printb(s->mask[s->mw*i+j], s->w % 32);
		putchar('\n');
	}
	putchar('\n');
}
