#include <SDL/SDL.h>
#include <stdlib.h>
#include "SFont.h"
#include "string.h"

SFont_FontInfo InternalFont;
int alpha=127;
int font_height;

Uint32 GetPixel(SDL_Surface *Surface, Sint32 X, Sint32 Y)
{

   Uint8  *bits;
   Uint32 Bpp;

   if (X<0) puts("SFONT ERROR: x too small in GetPixel. Report this to <karlb@gmx.net>");
   if (X>=Surface->w) puts("SFONT ERROR: x too big in GetPixel. Report this to <karlb@gmx.net>");
   
   Bpp = Surface->format->BytesPerPixel;

   bits = ((Uint8 *)Surface->pixels)+Y*Surface->pitch+X*Bpp;

   // Paint the walls with the fresh blood of your enemies

   // Get the pixel
   switch(Bpp) {
      case 1:
         return *((Uint8 *)Surface->pixels + Y * Surface->pitch + X);
         break;
      case 2:
         return *((Uint16 *)Surface->pixels + Y * Surface->pitch/2 + X);
         break;
      case 3: { // Format/endian independent 
         Uint8 r, g, b;
         r = *((bits)+Surface->format->Rshift/8);
         g = *((bits)+Surface->format->Gshift/8);
         b = *((bits)+Surface->format->Bshift/8);
         return SDL_MapRGB(Surface->format, r, g, b);
         }
         break;
      case 4:
         return *((Uint32 *)Surface->pixels + Y * Surface->pitch/4 + X);
         break;
   }

    return -1;
}

void InitFont2(SFont_FontInfo *Font)
{
    int x = 0, i = 0;

    if ( Font->Surface==NULL ) {
	printf("The font has not been loaded!\n");
	exit(1);
    }

    while ( x < Font->Surface->w ) {
	if(GetPixel(Font->Surface,x,0)==SDL_MapRGB(Font->Surface->format,255,0,255)) { 
    	    Font->CharPos[i++]=x;
    	    while (( x < Font->Surface->w-1) && (GetPixel(Font->Surface,x,0)==SDL_MapRGB(Font->Surface->format,255,0,255)))
		x++;
	    Font->CharPos[i++]=x;
	}
	x++;
    }

    Font->h=Font->Surface->h;
    font_height = Font->h;
    SDL_SetColorKey(Font->Surface, SDL_SRCCOLORKEY, GetPixel(Font->Surface, 0, Font->Surface->h-1));
}

void InitFont(SDL_Surface *Font)
{
    InternalFont.Surface=Font;
    InitFont2(&InternalFont);
}

int SFont_wide(char *text) {
    int i=0,xwide=0;
    int ofs;
    SFont_FontInfo *Font = &InternalFont;

    while (text[i]) {
        if (text[i]==' ') {
            xwide += (int)(Font->CharPos[2]-Font->CharPos[1]);
		} else {
			ofs = (text[i]-33)*2+1;
            xwide += (int)(Font->CharPos[ofs+1]-Font->CharPos[ofs]);
		}
		i++;
    }
    return xwide;
}

int SFont_height() {
    return InternalFont.Surface->h-1;
}

void PutString2(SDL_Surface *Surface, SFont_FontInfo *Font, int x, int y, char *text)
{
    int ofs;
    int i=0;
    SDL_Rect srcrect,dstrect; 

    while (text[i]) {
        if (text[i]==' ') {
            x+=Font->CharPos[2]-Font->CharPos[1];
	}
	else {
//	    printf("-%c- %c - %u\n",228,text[i],text[i]);
	    ofs=(text[i]-33)*2+1;
	    //printf("printing %c %d\n",text[i],ofs);
            srcrect.w = dstrect.w = (Font->CharPos[ofs+2]+Font->CharPos[ofs+1])/2-(Font->CharPos[ofs]+Font->CharPos[ofs-1])/2;
            srcrect.h = dstrect.h = Font->Surface->h-1;
            srcrect.x = (Font->CharPos[ofs]+Font->CharPos[ofs-1])/2;
            srcrect.y = 1;
    	    dstrect.x = x-(float)(Font->CharPos[ofs]-Font->CharPos[ofs-1])/2;
	    dstrect.y = y;

	    //SDL_SetAlpha ( Font->Surface, SDL_SRCALPHA, 127);
	    SDL_BlitSurface( Font->Surface, &srcrect, Surface, &dstrect); 

            x+=Font->CharPos[ofs+1]-Font->CharPos[ofs];
        }
        i++;
    }
}

// Return a new surface, with the text on it.
// This surface is new, fresh, and must eventually be freed.
// Create the new surface with the same colour system as a parent surface.
SDL_Surface *new_Surface_PutString(SDL_Surface *parent, char *text) {

     Uint32 rmask = parent->format->Rmask;
     Uint32 gmask = parent->format->Gmask;
     Uint32 bmask = parent->format->Bmask;
     Uint32 amask = parent->format->Amask;
     Uint32 bytesperpixel = parent->format->BytesPerPixel;

     return SDL_CreateRGBSurface(
	SDL_SWSURFACE, 
	SFont_wide(text), 
	SFont_height(), 
	bytesperpixel, rmask, gmask, bmask, amask
    );
}

void PutString(SDL_Surface *Surface, int x, int y, char *text) {
    PutString2(Surface, &InternalFont, x, y, text);
}

int TextWidth2(SFont_FontInfo *Font, char *text)
{
    int x=0,i=0;
    unsigned char ofs = 0;
    while (text[i]!='\0') {
        if (text[i]==' ') {
			x+=Font->CharPos[2]-Font->CharPos[1];
		} else {
			ofs=(text[i]-33)*2+1;
			x+=Font->CharPos[ofs+1]-Font->CharPos[ofs];
		}
		i++;
    }
    return x+Font->CharPos[ofs+2]-Font->CharPos[ofs+1];
}

int TextWidth(char *text)
{
    return TextWidth2(&InternalFont, text);
}

void TextAlpha(int a) {
    alpha = a;
}

void XCenteredString2(SDL_Surface *Surface, SFont_FontInfo *Font, int y, char *text)
{
    PutString2(Surface, &InternalFont, Surface->w/2-TextWidth(text)/2, y, text);
}

void XCenteredString(SDL_Surface *Surface, int y, char *text)
{
    XCenteredString2(Surface, &InternalFont, y, text);
}

SDL_Surface *Back;
char tmp[1024];

void clearBuffer() {
    SDL_Event event;

    // Delete the event buffer
    while (SDL_PollEvent(&event))
	;
    // start input
    SDL_EnableUNICODE(1);
}

int SFont_Input2( SDL_Surface *Dest, SFont_FontInfo *Font, int x, int y, int PixelWidth, char *text)
{
    SDL_Event event;
    int ch;
    SDL_Rect rect;
    int ofs=(text[0]-33)*2+1;
    int leftshift;

    if (ofs<0) {
	leftshift = 0;
    }
    else {
	leftshift = (Font->CharPos[ofs]-Font->CharPos[ofs-1])/2;
    }

    rect.x=x-leftshift;
    rect.y=y;
    rect.w=PixelWidth;
    rect.h=Font->Surface->h;

    //SDL_SetAlpha (Dest, SDL_SRCALPHA, 127);

    SDL_BlitSurface(Dest, &rect, Back, NULL);
    sprintf(tmp,"%s_",text);
    PutString2(Dest,Font,x,y,tmp);
    SDL_UpdateRect(Dest, x-leftshift, y, PixelWidth, Font->h);

    while (SDL_PollEvent(&event) && event.type==SDL_KEYDOWN) {

	// Find the character pressed
	ch=event.key.keysym.unicode;

	// If backspace and the length of the text > 0, reduce the string by 1 character
	if (ch=='\b') {
	    if (strlen(text)>0) {
		text[strlen(text)-1]='\0';
	    }
	}
	else {
	    sprintf(text,"%s%c",text,ch);
	}

	// If the new character would exceed the allowed width
	if (TextWidth2(Font,text)>PixelWidth) {
	    text[strlen(text)-1]='\0';
	}

	//SDL_SetAlpha (Back, SDL_SRCALPHA, 127);
	SDL_BlitSurface( Back, NULL, Dest, &rect);
	PutString2(Dest, Font, x, y, text);
	if (ofs>0) {
	    SDL_UpdateRect(Dest, x-(Font->CharPos[ofs]-Font->CharPos[ofs-1])/2, y, PixelWidth, Font->Surface->h);
	}

    }
    //text[strlen(text)-1]='\0';
    if (ch==SDLK_RETURN) {
	SDL_FreeSurface(Back);
	return 1;
    }
    else
	return 0;
}

int SFont_Input( SDL_Surface *Dest, int x, int y, int PixelWidth, char *text)
{
    
    Back = SDL_AllocSurface(Dest->flags,
    			    PixelWidth,
    			    InternalFont.h,
    			    Dest->format->BitsPerPixel,
    			    Dest->format->Rmask,
    			    Dest->format->Gmask,
			    Dest->format->Bmask, 0);

    return SFont_Input2( Dest, &InternalFont, x, y, PixelWidth, text);
    // ph:
    // Returns 1 when the return key is pressed
    // Returns 0 when nothing exceptional happened
}
