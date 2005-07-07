#ifndef VOR_CONFIG_H
#define VOR_CONFIG_H

#define VERSION "0.4pre"

// screen size
#define XSIZE 640
#define YSIZE 480

#define SCREENDXMIN 7.5

// image file containing font for score stuff.
#define BIG_FONT_FILE "fonts/score.png"


// -----------------------------------------------------------------------
// Rocks

// number of rock images
#define NROCKS 50 

// initial/final counts for rocks-on-screen
#define I_ROCKS 25
#define F_ROCKS 45

// number of rock structs to allocate
#define MAXROCKS 120

#define MAXENGINEDOTS 5000
#define MAXBANGDOTS 50000
#define W 100
#define M 255

// radius^2 (pixels) which will be cleared of rocks when you die
#define BLAST_RADIUS 300
#define DEAD_PAUSE_LENGTH 40.0

#define MAX_PATH_LEN 1024

#endif // VOR_CONFIG_H
