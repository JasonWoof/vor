#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "args.h"
#include <config.h>
#include "vorconfig.h"

// Look and Feel
int opt_fullscreen;
int opt_sound;
int opt_joystick_enabled;
int opt_joystick_number;
int opt_joystick_x_axis;
int opt_joystick_y_axis;

int opt_autopilot;

static void
show_help(void)
{
	puts("Dodge the rocks until you die.");
	putchar('\n');
	puts("  -f, --full-screen");
	puts("  -s, --silent             No explosion sounds or music");
	puts("  -j x,y, --joystick=x,y   set the axis numbers. defaults to 0,1");
	puts("                           press a joystick button to activate that joystick");
	puts("  -V, --version            Print program version");
	puts("  -?, --help               Give this help list");
	putchar('\n');
	puts("Report bugs at http://jasonwoof.com/contact.html");
}

// Advances buf while reading a positive int, terminated by [^0-9]
// buf is left pointing at first [^0-9]
// If digits are found, the parsed number is written to *out and 0 is returned.
// Otherwise *buf and *out are left unchanged, and non-zero is returned.
int
parse_next_int(char **buf, int* out) {
	int ret = 0, mul = 1;
	if(!*buf) {
		return 1;
	}
	// make sure *buf starts -?[0-9]
	if(**buf == '-') {
		mul = -1;
		if((*buf)[1] < '0' || (*buf)[1] > '9') {
			return 2;
		}
		*buf += 1;
	} else {
		if(**buf < '0' || **buf > '9') {
			return 2;
		}
	}
	while(**buf >= '0' && **buf <= '9') {
		ret *= 10;
		ret += **buf - '0';
		*buf += 1;
	}
	*out = ret * mul;

	return 0;
}

// returns 1 on success. return 0 causes usage message
int
parse_joystick_opts(char *arg) {
	char *arg_was = arg;
	static char* bad_arg = "Error: invalid argument to -j/--joystick.";

	// argument is required
	if(arg == NULL) {
		puts(bad_arg);
		return 0;
	}

	// read x axis
	if(parse_next_int(&arg, &opt_joystick_x_axis)) {
		puts(bad_arg);
		return 0;
	}

	// skip comma
	if(*arg != ',') {
		puts(bad_arg);
		return 0;
	}
	arg += 1;

	// read y axis
	if(parse_next_int(&arg, &opt_joystick_y_axis)) {
		puts(bad_arg);
		return 0;
	}

	// optionally joystick number
	if(*arg == ',') {
		arg += 1; // skip comma
		if(parse_next_int(&arg, &opt_joystick_number)) {
			puts(bad_arg);
			return 0;
		}
		opt_joystick_enabled = 1;
	}

	// end with a comma or end of string
	if(*arg != 0 && *arg != ',') {
		puts(bad_arg);
		return 0;
	}

	// mark arg as consumed (so it won't be parsed as a commandline switch)
	arg_was[0] = 0;

	// return success
	return 1;
}
int
short_opt(char c, char *arg)
{
	switch(c) {
		case 'f': opt_fullscreen = 1; break;
		case 's': opt_sound = 0; break;
		case 'j':
			return parse_joystick_opts(arg);
		case 'V':
				  printf("Variations on Rockdodger %s\n", PACKAGE_VERSION);
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

static char *long_opts[] = { "full-screen", "silent", "joystick", "version", "help", "autopilot" };

static char short_opts[] = { 'f', 's', 'j', 'V', 'h', 'a' };

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
	opt_joystick_enabled = 0; // can also be enabled by pressing one of its buttons
	opt_joystick_number = 0;
	opt_joystick_x_axis = 0;
	opt_joystick_y_axis = 1;
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
		// args that are consumed as arguments to an option (such as the zero
		// in: -j 0) have their first byte set to null. So skip them:
		s = argv[i]; if(!*s) continue;
		if(*s++ != '-') {
			fputs("not an option\n\n", stderr);
			show_help();
			return 0;
		}

		arg = NULL;
		for(r=s; *r; r++) if(*r == '=') { *r = 0; arg = r+1; break; }

		if(arg == NULL && i + 1 < argc) {
			arg = argv[i+1];
			// if this is used, it's first byte will be set to null, and
			// it'll be skipped. See above.
		}

		if(*s == '-') {
			if(!parse_long_opt(s+1, arg)) { show_help(); return 0; }
		} else {
		   if(!parse_short_opts(s, arg)) { show_help(); return 0; }
		}
	}
	return 1;
}
