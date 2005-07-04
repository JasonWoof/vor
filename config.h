#ifndef VOR_CONFIG_H
#define VOR_CONFIG_H

#define debug(x) if(DEBUG) { x; }

#define VERSION "0.3"
#define DATA_PREFIX "/usr/share/vor"

// screen size
#define XSIZE 640
#define YSIZE 480

// number of rock image files.
#define NROCKS 50
// image file containing font for score stuff.
#define BIG_FONT_FILE "fonts/score.png"
#define I_ROCKS 25  // initial/final counts for rocks-on-screen.
#define F_ROCKS 45

#define MAXROCKS 120 // MAX Rocks
#define ROCKRATE 2
#define MAXENGINEDOTS 5000
#define MAXBANGDOTS 50000
#define MAXSPACEDOTS 2000
#define MAXDUSTDEPTH 2
#define W 100
#define M 255
#define BLAST_RADIUS 300 // radius^2 (pixels) which will be cleared of rocks when you die
#define DEAD_PAUSE_LENGTH 40.0

#define MAX_PATH_LEN 1024

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define CONDERROR(a) if((a)) {initerror = strdup(SDL_GetError());return 1;}
#define NULLERROR(a) CONDERROR((a) == NULL)

#endif // VOR_CONFIG_H
