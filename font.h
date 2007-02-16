//  Copyright (C) 2006 Jason Woofenden  PUBLIC DOMAIN

#ifndef __font_h__
#define __font_h__

typedef struct {
	SDL_Surface *pixels;	
	SDL_Rect bounds[94];
	int space_width;
	int letter_spacing;
} font;

font *font_load(const char* filename); // sets as the current font
void font_free(font*);
void font_set(font*);
void font_write(int x, int y, const char* message);
int font_width(const char* message);
int font_height();

#endif
