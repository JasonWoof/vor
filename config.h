#ifndef VOR_CONFIG_H
#define VOR_CONFIG_H

#define VERSION "0.5.3"

// screen size
#define XSIZE 640
#define YSIZE 480

#define XSCROLLTO (XSIZE/3)
#define YSCROLLTO (YSIZE/2)

#define BARRIER_SPEED 7.5

#define MAX_DIST_AHEAD XSIZE
#define BOUNCINESS 0.50


// -----------------------------------------------------------------------
// Rocks

// number of rock images
#define NROCKS 50 

// initial/final counts for rocks-on-screen
#define NORMAL_I_ROCKS 20
#define NORMAL_F_ROCKS 35  // after 2 minutes
#define NORMAL_GAMESPEED 1.0

#define EASY_I_ROCKS 10
#define EASY_F_ROCKS 25  // after 2 minutes
#define EASY_GAMESPEED 0.85

// number of rock structs to allocate
#define MAXROCKS 120

#define THRUSTER_STRENGTH 1.5

// how many engine dots come out of each thruster per tic (1/20th of a seccond)
#define ENGINE_DOTS_PER_TIC 150

#define MAXENGINEDOTS 10000
#define MAXBANGDOTS 50000
#define W 100
#define M 255

// determines how hard they push the rocks. Set to 0 to disable pushing rocks
#define DOT_MASS_UNIT 0.07

// radius^2 (pixels) which will be cleared of rocks when you die
#define BLAST_RADIUS 300

// time (in 1/60ths of a seccond) between when you blow up, and when your next
// ship appears. Make it at least 20.0 so the explosion has time to push the
// rocks away.
#define DEAD_PAUSE_LENGTH 40.0

#define MAX_PATH_LEN 1024

#endif // VOR_CONFIG_H
