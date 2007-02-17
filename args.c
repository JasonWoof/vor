#include <string.h>
#include <stdlib.h>
#include <stdio.h>
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

static void
show_help(void)
{
	puts("Dodge the rocks until you die.");
	putchar('\n');
	puts(" Gameplay Variations:");
    puts("  -b, --bounciness=N%        Keep N% of speed when hitting edges (default 50%)");
	puts("  -g, --game-speed=N%        50-200% (default 100%)");
	putchar('\n');
	puts(" Look and Feel:");
	puts("  -f, --full-screen");
	puts("  -m, --music                Enable music");
	puts("  -s, --silent               No explosion sounds or music");
	putchar('\n');
	puts(" Informational:");
	puts("  -?, --help                 Give this help list");
	puts("  -V, --version              Print program version");
	putchar('\n');
	puts("Mandatory or optional arguments to long options are also mandatory or optional");
	puts("for any corresponding short options.");
	putchar('\n');
	puts("Report bugs at http://jasonwoof.com/contact.html");
}

int
short_opt(char c, char *arg)
{
	int i;

	switch(c) {
		case 'b': if(!arg || sscanf(arg, "%d%%", &i) != 1 || i < 0 || i > 100) {
					  fprintf(stderr, "bad --bounciness (-b) value (should be 0-100%%)\n\n");
					  return 0;
				  }
				  opt_bounciness = (float)i / 100;
				  *arg = 0;
				  break;
		case 'f': opt_fullscreen = 1; break;
		case 'g': if(!arg || sscanf(arg, "%d%%", &i) != 1 || i < 50 || i > 200) {
					  fprintf(stderr, "bad --game-speed (-g) value (should be 50-200%%)\n\n");
					  return 0;
				  }
				  opt_gamespeed = (float)i / 100;
				  *arg = 0;
				  break;
		case 'l': if(sscanf(arg, "%f", &opt_max_lead) != 1) {
					  fprintf(stderr, "bad --max-lead (-l) value (must be a number)\n\n");
					  return 0;
				  }
				  opt_max_lead *= XSIZE;
				  *arg = 0;
				  break;
		case 'm': opt_music = 1; break;
		case 's': opt_sound = 0; opt_music = 0; break;
		case 'V':
				  printf("Variations on Rockdodger %s\n", VERSION);
				  exit(0);
		case '?':
		case 'h': return 0;
		default: 
				  fprintf(stderr, "unknown option -%c\n\n", c);
				  return 0;
	}
	return 1;
}

int
parse_short_opts(const char *s, char *arg)
{
	while(s[1]) if(!short_opt(*s++, NULL)) return 0;
	return short_opt(*s, arg);
}

static char *long_opts[] = {
	"bounciness", "game-speed",
	"full-screen", "music", "silent",
	"help", "version"
};

static char short_opts[] = {
	'b', 'g',
	'f', 'm', 's',
	'h', 'V'
};

int
parse_long_opt(const char *s, char *arg)
{
	int i;
	for(i=0; i<sizeof(short_opts); i++) {
		if(strcmp(s, long_opts[i]) == 0)
			return short_opt(short_opts[i], arg);
	}
	fprintf(stderr, "unknown long option --%s\n\n", s);
	return 0;
}

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
	opt_music = 1;
}

int
parse_opts(int argc, char *argv[])
{
	int i;
	char *r;

	init_opts();
	for(i=1; i<argc; i++) {
		char *s, *arg;
		s = argv[i]; if(!*s) continue;
		if(*s++ != '-') {
			fputs("not an option\n\n", stderr);
			show_help();
			return 0;
		}

		arg = NULL;
		for(r=s; *r; r++) if(*r == '=') { *r = 0; arg = r+1; break; }
		if(!arg && (i+1 < argc)) arg = argv[i+1];

		if(*s == '-') {
			if(!parse_long_opt(s+1, arg)) { show_help(); return 0; }
		} else {
		   if(!parse_short_opts(s, arg)) { show_help(); return 0; }
		}
	}
	return 1;
}
