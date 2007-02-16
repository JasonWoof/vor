//  Copyright (C) 2006 Jason Woofenden
//
//  This file is part of VoR.
//
//  VoR is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2, or (at your option)
//  any later version.
//
//  VoR is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with VoR; see the file COPYING.  If not, write to the
//  Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//  MA 02111-1307, USA.

#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

#include "font.h"

font *cur_font;
extern SDL_Surface *surf_screen;

int font_height() {
	return cur_font->bounds[0].h;
}

void font_set(font *to) {
	cur_font = to;
}

// return true if column of pixels (pix points to the top one) is all black
int line_clear(unsigned char* pix, int height, int width) {
	int i;
	for(i = 0; i < height; ++i) {
		if(pix[0] != 0) {
			return 0;
		}
		pix += width;
	}
	
	return 1;
}


// return the number of consecutive colums of pixels are still clear (or still not clear)
int find_change(unsigned char* pix, int height, int width) {
	int i;
	int state = line_clear(pix, height, width);
	pix += 3;

	for(i = 1; /*forever*/; pix += 3, ++i) {
		if(line_clear(pix, height, width) != state) {
			return i;
		}
	}
}

void font_free(font* garbage) {
	SDL_FreeSurface(garbage->pixels);
	free(garbage);
}

font* font_load(const char *filename) {
	font* new_font;
	int i;
	unsigned char* pix;
	int x = 0;
	int width;

	new_font = (font*) malloc(sizeof(font));
	if(!new_font) {
		fprintf(stderr, "couldn't allocate memory for font.\n");
		exit(1);
	}
	new_font->pixels = (SDL_Surface*)IMG_Load(filename);
	if(!new_font->pixels) {
		fprintf(stderr, "couldn't load font file '%s'.\n", filename);
		exit(1);
	}

	//SDL_SetColorKey(new_font->pixels, SDL_SRCCOLORKEY, 0);

	pix = new_font->pixels->pixels;

	// set all font rects to be the full height
	for(i = 0; i < 94; ++i) {
		new_font->bounds[i].y = 0;
		new_font->bounds[i].h = new_font->pixels->h;
	}

	// find the characters
	new_font->bounds[0].x = 0; // the first character starts at the begining
	for(i = 0; i < 93; ) {
		// find the end of the character
		width = find_change(pix, new_font->pixels->h, new_font->pixels->pitch);
		x += width;
		pix += width * new_font->pixels->format->BytesPerPixel;
		new_font->bounds[i].w = width;

		++i;

		width = find_change(pix, new_font->pixels->h, new_font->pixels->pitch);
		x += width;
		pix += width * new_font->pixels->format->BytesPerPixel;
		new_font->bounds[i].x = x;
	}
	new_font->bounds[93].w = new_font->pixels->w - new_font->bounds[93].x; // the last character ends at the end

	// There is a common problem where with some fonts there is a column of all
	// black pixels between the parts of the double-quote, and this code thinks
	// that it is two seperate characters. This code currently assumes that
	// there is no such column. To change it so it assumes that the
	// double-quote character looks like 2, change the loop above to start at
	// zero, and uncomment the following indented stuff.

			//	// The above has 3 problems:
			//
			//	// 1) space is missing from the begining
			//	// 2) ! is first instead of seccond
			//	// 3) " is taking 2nd and 3rd position
			//
			//	// merge pieces of double-quote
			//	width = font_rects[2].x - font_rects[1].x;
			//	font_rects[2].w += width;
			//	font_rects[2].x = font_rects[1].x;
			//
			//	// move !
			//	font_rects[1].x = font_rects[0].x;
			//	font_rects[1].w = font_rects[0].w;


	// the width of the space is set to half the space between the first two characters
	width = new_font->bounds[1].x - (new_font->bounds[0].x + new_font->bounds[0].w);
	new_font->space_width = width / 2;
	new_font->letter_spacing = new_font->space_width / 4 ;
	if(new_font->space_width < 2) {
		new_font->space_width = 2;
	}
	if(new_font->letter_spacing < 1) {
		new_font->letter_spacing = 1;
	}

	font_set(new_font);
	return new_font;
}

void font_write(int x, int y, const char* message) {
	SDL_Rect dest = {x, y, 0, font_height()};
	char c;

	if(message[0] == 0) {
		return;
	}

	for(c = *message++; c; c = *message++) {
		if(c > 31 && c < 127) {
			if(c == 32) {
				dest.x += cur_font->space_width;
			} else {
				c -= 33;
				dest.w = cur_font->bounds[(int)c].w;
				SDL_BlitSurface(cur_font->pixels, &(cur_font->bounds[(int)c]), surf_screen, &dest);
				dest.x += dest.w;
			}
			dest.x += cur_font->letter_spacing;
		} else {
			fprintf(stderr, "tried to print unknown char: %d (0x%x)\n", c, c);
		}
	}
}

// return the width in pixels of the string
int font_width(const char* message) {
	int width = 0;
	char c;

	if(message[0] == 0) {
		return 0;
	}

	for(c = *message++; c; c = *message++) {
		if(c > 31 && c < 127) {
			if(c == 32) {
				width += cur_font->space_width;
			} else {
				c -= 33;
				width += cur_font->bounds[(int)c].w;
			}
			width += cur_font->letter_spacing;
		} else {
			fprintf(stderr, "tried to print unknown char: %d (0x%x)\n", c, c);
		}
	}

	// don't count spacing after the last char
	if(width) {
		width -= cur_font->letter_spacing;
	}

	return width;
}
