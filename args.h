#ifndef VOR_ARGS_H
#define VOR_ARGS_H

#include <argp.h>

// Gameplay Variations
extern float opt_bounciness;
extern float opt_gamespeed;
extern float opt_max_lead;

// Look and Feel
extern int opt_fullscreen;
extern int opt_music;
extern int opt_sound;

int parse_opts(int argc, char *argv[]);

#endif // VOR_ARGS_H
