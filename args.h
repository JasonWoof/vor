#ifndef VOR_ARGS_H
#define VOR_ARGS_H

#include <argp.h>

extern int opt_fullscreen;
extern int opt_music;
extern int opt_sound;
extern float opt_bounciness;
extern float opt_gamespeed;
extern int opt_tail_engine;
extern int opt_friction;

struct argp argp;

void init_opts(void);

#endif // VOR_ARGS_H
