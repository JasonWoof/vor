#include "args.h"
#include "config.h"

int opt_fullscreen;
int opt_music;
int opt_sound;
float opt_bounciness;
float opt_gamespeed;
int opt_tail_engine;
int opt_friction;

error_t parse_opt(int, char*, struct argp_state *);

const char *argp_program_version = "Variations on Rockdodger " VERSION;
const char *argp_program_bug_address = "<josh@qualdan.com>";
static char doc[] = "Dodge the rocks until you die.";
static struct argp_option opts[] = {
	{0, 0, 0, 0, "Gameplay Variations:"},
	{"bounciness", 'b', "N%", 0, "keep N% of speed when hitting edges (default 50%)"},
	{"game-speed", 'g', "N%", 0, "50-100% (default 100%)"},
	{"bad-physics", 'p', 0, 0, "bad physics (i.e. friction)"},
	{0, 0, 0, 0, "Look and Feel:"},
	{"engine", 'e', 0, 0, "Display large tail plume"},
	{"full-screen", 'f', 0, 0, ""},
	{"music", 'm', 0, 0, "Enable music"},
	{"silent", 's', 0, 0, "Turn off explosion sounds"},
	{0, 0, 0, 0, "Informational:", -1},
	{0}
};

struct argp argp = { opts, &parse_opt, 0, doc };

void
init_opts(void)
{
	opt_fullscreen = 0;
	opt_sound = 1;
	opt_music = 0;
	opt_bounciness = 0.50; // lose 50% when you hit the screen edge.
	opt_gamespeed = 1.00; // Run game at full speed.
	// These switch back to the old gameplay and are off by default.
	opt_tail_engine = 0;
	opt_friction = 0;
}

error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
	int i;

	switch(key) {
		case 'f': opt_fullscreen = 1; break;
		case 'm': opt_music = 1; break;
		case 's': opt_sound = 0; opt_music = 0; break;
		case 'b': i = 0; sscanf(arg, "%d%%", &i);
				  if(i < 50) i = 50; else if(i > 100) i = 100;
				  opt_bounciness = (float)i / 100;
				  break;
		case 'g': i = 0; sscanf(arg, "%d%%", &i);
				  if(i < 0) i = 0; else if(i > 100) i = 100;
				  opt_gamespeed = (float)i / 100;
				  break;
		case 'e': opt_tail_engine = 1; break;
		case 'p': opt_friction = 1; break;
		default: break;
	}
	return 0;
}
