//  Copyright (C) 2007 Jason Woofenden
//
//  This file is part of VoR.
//
//  VoR is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2, or (at your option)
//  any later version.
//
//  VoR is distributed in the hope that it will be useful, but
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with VoR; see the file COPYING.  If not, write to the
//  Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//  MA 02111-1307, USA.


// This file has code to generate SDL keypress events

#include <stdlib.h>
#include <SDL.h>
#include "mt.h"

// SETTINGS
//
// auto pilot waits a random amount of time between zero and this many 1/20ths of a second
#define AUTOPILOT_MAX_DELAY 20.0

#define NUM_KEYS 5
SDLKey keysyms[NUM_KEYS] = { SDLK_SPACE, SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN};
int states[NUM_KEYS] =     { 0,          0,         0,          0,       0,       };

float next_time;
float delay;

void
autopilot_set_timer() {
	delay = 0;
	next_time = frnd() * AUTOPILOT_MAX_DELAY;
}

void
autopilot_init() {
	autopilot_set_timer();
}
	
void
autopilot_fix_keystates(Uint8* table) {
	int i;
	for(i = 0; i < NUM_KEYS; ++i) {
		table[keysyms[i]] = states[i];
	}
}

int
autopilot_num_down() {
	int i, num;
	for(num = 0, i = 0; i < NUM_KEYS; ++i) {
		if(states[i]) {
			num++;
		}
	}

	return num;
}

void
autopilot_send_event() {
	int down = autopilot_num_down();
	int nth, i;
	//SDL_Event event;

	if(down && frnd() < 0.25 + (down / 4.0)) {
		//event.type = SDL_KEYUP;
		//event.key.state = 0;
		nth = (urnd() % down) + 1;
		for(i = 0; ; ++i) {
			if(states[i]) {
				--nth;
			}
			if(nth == 0) {
				//event.key.keysym.sym = keysyms[i];
				states[i] = 0;
				break;
			}
		}
	} else {
		//event.type = SDL_KEYDOWN;
		//event.key.state = 1;
		nth = (urnd() % (NUM_KEYS - down)) + 1;
		for(i = 0; ; ++i) {
			if(!states[i]) {
				--nth;
			}
			if(nth == 0) {
				//event.key.keysym.sym = keysyms[i];
				states[i] = 1;
				break;
			}
		}
	}

	//fprintf(stderr, "push event: %i %i\n", event.key.keysym.sym, event.type);
	//SDL_PushEvent(&event);
}

void
autopilot(float ticks) {
	delay += ticks;
	if(delay > next_time) {
		autopilot_set_timer();
		autopilot_send_event();
	}
}
