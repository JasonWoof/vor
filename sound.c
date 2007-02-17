#include <SDL.h>
#include <SDL_mixer.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "args.h"
#include "common.h"
#include "config.h"
#include "sound.h"


static Mix_Music *music[NUM_TUNES];
static int music_volume[NUM_TUNES] = {128,128,128};
static Mix_Chunk *wav[NUM_SOUNDS];

int audio_rate;
Uint16 audio_format;
int audio_channels;

char *add_data_path(char *);
char *wav_file[] = {
	"booom.wav",
	"cboom.wav",
	"boom.wav",
	"bzboom.wav"
};

char *tune_file[] = {
	"magic.mod",
	"getzznew.mod",
	"4est_fulla3s.mod"
};

// Return 1 if the sound is ready to roll, and 0 if not.
int
init_sound() {
	int i;
	char *s;

	// Initialise output with SDL_mixer
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 4096) < 0) {
	fprintf(stderr, "Couldn't open SDL_mixer audio: %s\n", SDL_GetError());
	return 0;
	}

	// Preload all the tunes into memory
	for (i=0; i<NUM_TUNES; i++) {
		s = add_data_path(tune_file[i]);
		if(s) {
			music[i] = Mix_LoadMUS(s);
			if(!music[i]) printf("Failed to load %s.\n", s);
			free(s);
		}
	}

	// Preload all the wav files into memory
	for (i=0; i<NUM_SOUNDS; i++) {
		s = add_data_path(wav_file[i]);
		if(s) {
			wav[i] = Mix_LoadWAV(s);
			free(s);
		}
	}

	return 1;
}

void
play_sound(int i)  {
	if(!opt_sound) return;
	Mix_PlayChannel(-1, wav[i], 0);
}

int playing=-1;


void
play_tune(int i) {
	if(!opt_sound) return;
	if (playing==i)
	return;
	if (playing) {
		Mix_FadeOutMusic(1500);
	}
	Mix_FadeInMusic(music[i],-1,2000);
	Mix_VolumeMusic(music_volume[i]);

	playing = i;
}

/*
 *
 * The init_sound() routine is called first.
 * The play_sound() routine is called with the index number of the sound we wish to play.
 * The play_tune() routine is called with the index number of the tune we wish to play.
 *
 */
