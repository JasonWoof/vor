#include <errno.h>
#include "args.h"
#include "config.h"

// Gameplay Variations
float opt_bounciness;
float opt_gamespeed;
float opt_max_lead;
int opt_friction;

// Look and Feel
int opt_fullscreen;
int opt_music;
int opt_sound;
int opt_tail_engine;

error_t parse_opt(int, char*, struct argp_state *);

const char *argp_program_version = "Variations on Rockdodger " VERSION;
const char *argp_program_bug_address = "<josh@qualdan.com>";
static char doc[] = "Dodge the rocks until you die.";
static struct argp_option opts[] = {
	{0, 0, 0, 0, "Gameplay Variations:"},
	{"bounciness", 'b', "N%", 0, "Keep N% of speed when hitting edges (default 50%)"},
	{"game-speed", 'g', "N%", 0, "50-100% (default 100%)"},
	{"max-lead", 'l', "#SCREENS", 0, "Max dist. ahead you can get (default 1 screen)\n(negative value means no limit)"},
	{"bad-physics", 'p', 0, 0, "Bad physics (i.e. friction)"},
	{0, 0, 0, 0, "Look and Feel:"},
	{"engine", 'e', 0, 0, "Display large tail plume"},
	{"full-screen", 'f', 0, 0, ""},
	{"music", 'm', 0, 0, "Enable music"},
	{"silent", 's', 0, 0, "No explosion sounds or music"},
	{0, 0, 0, 0, "Informational:", -1},
	{0}
};

struct argp argp = { opts, &parse_opt, 0, doc };

void
init_opts(void)
{
	// Gameplay Variations
	opt_bounciness = 0.50; // lose 50% when you hit the screen edge.
	opt_gamespeed = 1.00; // Run game at full speed.
	opt_max_lead = 1.00*XSIZE;  // you can get 1 screen ahead.
	opt_friction = 0;

	// Look and Feel
	opt_fullscreen = 0;
	opt_sound = 1;
	opt_music = 0;
	opt_tail_engine = 0;
}

error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
	int i;

	switch(key) {
		case 'b': if(!sscanf(arg, "%d%%", &i)) {
					  argp_error(state, "bad --bounciness (-b) value (should be 0-100%%)");
					  return EINVAL;
				  }
				  if(i < 50) i = 50; else if(i > 100) i = 100;
				  opt_bounciness = (float)i / 100;
				  break;
		case 'e': opt_tail_engine = 1; break;
		case 'f': opt_fullscreen = 1; break;
		case 'g': if(!sscanf(arg, "%d%%", &i)) {
					  argp_error(state, "bad --gamespeed (-g) value (should be 50-100%%)");
					  return EINVAL;
				  }
				  if(i < 0) i = 0; else if(i > 100) i = 100;
				  opt_gamespeed = (float)i / 100;
				  break;
		case 'l': if(!sscanf(arg, "%f", &opt_max_lead)) {
					  argp_error(state, "bad --max-limit (-l) value (must be a number)");
					  return EINVAL;
				  }
				  opt_max_lead *= XSIZE;
				  break;
		case 'm': opt_music = 1; break;
		case 'p': opt_friction = 1; break;
		case 's': opt_sound = 0; opt_music = 0; break;
		default: break;
	}
	return 0;
}
