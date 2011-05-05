#ifndef VOR_ARGS_H
#define VOR_ARGS_H

// Look and Feel
extern int opt_fullscreen;
extern int opt_sound;
extern int opt_joystick_enabled;
extern int opt_joystick_number;
extern int opt_joystick_x_axis;
extern int opt_joystick_y_axis;

extern int opt_autopilot;

int parse_opts(int argc, char *argv[]);

#endif // VOR_ARGS_H
