#ifndef VOR_CONFIG_H
#define VOR_CONFIG_H

#define VERSION "0.4pre"

// screen size
#define XSIZE 640
#define YSIZE 480

#define XSCROLLTO (XSIZE/3)
#define YSCROLLTO (YSIZE/2)

#define SCREENDXMIN 7.5

// image file containing font for score stuff.
#define BIG_FONT_FILE "fonts/score.png"


// -----------------------------------------------------------------------
// Rocks

// number of rock images
#define NROCKS 50 

// initial/final counts for rocks-on-screen
#define I_ROCKS 20
#define F_ROCKS 40

// number of rock structs to allocate
#define MAXROCKS 120

#define MAXENGINEDOTS 5000
#define MAXBANGDOTS 50000
#define W 100
#define M 255

// radius^2 (pixels) which will be cleared of rocks when you die
#define BLAST_RADIUS 300

// time (in 1/60ths of a seccond) between when you blow up, and when your next
// ship appears. Make it at least 20.0 so the explosion has time to push the
// rocks away.
#define DEAD_PAUSE_LENGTH 40.0

#define MAX_PATH_LEN 1024

#endif // VOR_CONFIG_H
