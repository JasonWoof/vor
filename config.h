#ifndef VOR_CONFIG_H
#define VOR_CONFIG_H

#define debug(x) if(DEBUG) { x; }

#define VERSION "0.1"
#define DATA_PREFIX "/usr/share/vor"

// screen size
#define XSIZE 640
#define YSIZE 480

// number of rock image files.
#define NROCKS 6
// image file containing font for score stuff.
#define BIG_FONT_FILE "fonts/score.png"

#define MAXROCKS 120 // MAX Rocks
#define MAXROCKHEIGHT 100
#define ROCKRATE 2
#define MAXBLACKPOINTS 500
#define MAXENGINEDOTS 5000
#define MAXBANGDOTS 50000
#define MAXSPACEDOTS 2000
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

#endif // VOR_CONFIG_H
