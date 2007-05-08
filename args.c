#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "args.h"
#include "config.h"

// Look and Feel
int opt_fullscreen;
int opt_sound;

int opt_autopilot;

static void
show_help(void)
{
	puts("Dodge the rocks until you die.");
	putchar('\n');
	puts("  -f, --full-screen");
	puts("  -s, --silent               No explosion sounds or music");
	puts("  -V, --version              Print program version");
	puts("  -?, --help                 Give this help list");
	putchar('\n');
	puts("Mandatory or optional arguments to long options are also mandatory or optional");
	puts("for any corresponding short options.");
	putchar('\n');
	puts("Report bugs at http://jasonwoof.com/contact.html");
}

int
short_opt(char c, char *arg)
{
	switch(c) {
		case 'f': opt_fullscreen = 1; break;
		case 's': opt_sound = 0; break;
		case 'V':
				  printf("Variations on Rockdodger %s\n", VERSION);
				  exit(0);
		case '?':
		case 'h': return 0;
		case 'a': opt_autopilot = 1; break;
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

static char *long_opts[] = { "full-screen", "silent", "version", "help", "autopilot" };

static char short_opts[] = { 'f', 's', 'V', 'h', 'a' };

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
	opt_fullscreen = 0;
	opt_sound = 1;
	opt_autopilot = 0;
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
