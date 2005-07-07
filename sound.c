#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
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

char *add_path(char *);
char *wav_file[] = {
	"sounds/booom.wav",
	"sounds/cboom.wav",
	"sounds/boom.wav",
	"sounds/bzboom.wav"
};

char *tune_file[] = {
	"music/magic.mod",
	"music/getzznew.mod",
	"music/4est_fulla3s.mod"
};

int
init_sound() {
	// Return 1 if the sound is ready to roll, and 0 if not.

	int i;
	debug(printf ("Initialise sound\n"));

	// Initialise output with SDL_mixer
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 4096) < 0) {
	fprintf(stderr, "Couldn't open SDL_mixer audio: %s\n", SDL_GetError());
	return 0;
	}

	debug(
			// What kind of sound did we get?  Ah who cares. As long as it can play
			// some basic bangs and simple music.
			Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
			printf("Opened audio at %d Hz %d bit %s\n", audio_rate,
				(audio_format&0xFF),
				(audio_channels > 1) ? "stereo" : "mono");
			) 

	// Preload all the tunes into memory
	for (i=0; i<NUM_TUNES; i++) {
		if (!(music[i] = Mix_LoadMUS(add_path(tune_file[i])))) {
			printf ("Failed to load %s\n",add_path(tune_file[i]));
		}
	}

	// Preload all the wav files into memory
	for (i=0; i<NUM_SOUNDS; i++) {
		wav[i] = Mix_LoadWAV(add_path(wav_file[i]));
	}

	return 1;
}

void
play_sound(int i)  {
	if(!opt_sound) return;
	debug(printf ("play sound %d on first free channel\n",i));
	Mix_PlayChannel(-1, wav[i], 0);
}/*}}}*/

int playing=-1;


void
play_tune(int i) {/*{{{*/
	if(!opt_music) return;
	if (playing==i)
	return;
	if (playing) {
		Mix_FadeOutMusic(1500);
		debug(printf("Stop playing %d\n",playing));
	}
	debug(
			printf ("Play music %d\n",i);
			printf ("volume %d\n",music_volume[i]);
			)
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
