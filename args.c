#include <errno.h>
#include "args.h"
#include "config.h"

// Gameplay Variations
float opt_bounciness;
float opt_gamespeed;
float opt_max_lead;

// Look and Feel
int opt_fullscreen;
int opt_music;
int opt_sound;

error_t parse_opt(int, char*, struct argp_state *);

const char *argp_program_version = "Variations on Rockdodger " VERSION;
const char *argp_program_bug_address = "<josh@qualdan.com>";
static char doc[] = "Dodge the rocks until you die.";
static struct argp_option opts[] = {
	{0, 0, 0, 0, "Gameplay Variations:"},
	{"bounciness", 'b', "N%", 0, "Keep N% of speed when hitting edges (default 50%)"},
	{"game-speed", 'g', "N%", 0, "50-100% (default 100%)"},
	{"max-lead", 'l', "#SCREENS", OPTION_HIDDEN,
		"Max distance ahead you can get\n (default 1 screen; < 0 means no limit)"},
	{0, 0, 0, 0, "Look and Feel:"},
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

	// Look and Feel
	opt_fullscreen = 0;
	opt_sound = 1;
	opt_music = 0;
}

error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
	int i;

	switch(key) {
		case 'b': if(!sscanf(arg, "%d%%", &i)) {
					  fprintf(stderr, "bad --bounciness (-b) value (should be 0-100%%)\n");
					  argp_state_help(state, stderr, ARGP_HELP_STD_HELP);
					  return EINVAL;
				  }
				  if(i < 50) i = 50; else if(i > 100) i = 100;
				  opt_bounciness = (float)i / 100;
				  break;
		case 'f': opt_fullscreen = 1; break;
		case 'g': if(!sscanf(arg, "%d%%", &i)) {
					  fprintf(stderr, "bad --gamespeed (-g) value (should be 50-100%%)\n");
					  argp_state_help(state, stderr, ARGP_HELP_STD_HELP);
					  return EINVAL;
				  }
				  if(i < 0) i = 0; else if(i > 100) i = 100;
				  opt_gamespeed = (float)i / 100;
				  break;
		case 'l': if(!sscanf(arg, "%f", &opt_max_lead)) {
					  fprintf(stderr, "bad --max-lead (-l) value (must be a number)\n");
					  argp_state_help(state, stderr, ARGP_HELP_STD_HELP);
					  return EINVAL;
				  }
				  opt_max_lead *= XSIZE;
				  break;
		case 'm': opt_music = 1; break;
		case 's': opt_sound = 0; opt_music = 0; break;
		case ARGP_KEY_END:
				  break;
		default:
				  return ARGP_ERR_UNKNOWN;
	}
	return 0;
}
