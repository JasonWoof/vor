#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#define CONDERROR(a) if ((a)) {fprintf(stderr,"Error: %s\n",SDL_GetError());exit(1);}
#define NULLERROR(a) CONDERROR((a)==NULL)

#define TUNE_TITLE_PAGE		0
#define TUNE_GAMEPLAY		1
#define TUNE_HIGH_SCORE_ENTRY	2
#define NUM_TUNES		3

#define SOUND_BANG		0
#define NUM_SOUNDS		4

static Mix_Music *music[NUM_TUNES];
static int music_volume[NUM_TUNES] = {128,128,128};
static Mix_Chunk *wav[NUM_SOUNDS];

int audio_rate;
Uint16 audio_format;
int audio_channels;

char *load_file(char *);
char *wav_file[] = {
    "sounds/booom.wav",
	"sounds/cboom.wav",
	"sounds/boom.wav",
	"sounds/bzboom.wav"
};

char *tune_file[] = {/*{{{*/
    "music/magic.mod",
    "music/getzznew.mod",
    "music/4est_fulla3s.mod"
};/*}}}*/

int init_sound() {/*{{{*/
    // Return 1 if the sound is ready to roll, and 0 if not.

    int i;
#ifdef DEBUG
    printf ("Initialise sound\n");
#endif

    // Initialise output with SDL_mixer
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, AUDIO_S16, MIX_DEFAULT_CHANNELS, 4096) < 0) {
	fprintf(stderr, "Couldn't open SDL_mixer audio: %s\n", SDL_GetError());
	return 0;
    }

#ifdef DEBUG
    // What kind of sound did we get?  Ah who cares. As long as it can play
    // some basic bangs and simple music.
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
    printf("Opened audio at %d Hz %d bit %s\n", audio_rate,
	    (audio_format&0xFF),
	    (audio_channels > 1) ? "stereo" : "mono");
#endif

    // Preload all the tunes into memory
    for (i=0; i<NUM_TUNES; i++) {
	if (!(music[i] = Mix_LoadMUS(load_file(tune_file[i])))) {
	    printf ("Failed to load %s\n",load_file(tune_file[i]));
	}
    }

    // Preload all the wav files into memory
    for (i=0; i<NUM_SOUNDS; i++) {
	wav[i] = Mix_LoadWAV(load_file(wav_file[i]));
    }

    return 1;
}/*}}}*/

void play_sound(int i)  {/*{{{*/
#ifdef DEBUG
    printf ("play sound %d on first free channel\n",i);
#endif
    Mix_PlayChannel(-1, wav[i], 0);
}/*}}}*/

int playing=-1;


#undef DEBUG

void play_tune(int i) {/*{{{*/
    if (playing==i)
	return;
    if (playing) {
	Mix_FadeOutMusic(1500);
#ifdef DEBUG
	printf("Stop playing %d\n",playing);
#endif
    }
#ifdef DEBUG
    printf ("Play music %d\n",i);
    printf ("volume %d\n",music_volume[i]);
#endif
    Mix_FadeInMusic(music[i],-1,2000);
    Mix_VolumeMusic(music_volume[i]);

    playing = i;
}/*}}}*/

/*
 *
 * The init_sound() routine is called first.
 * The play_sound() routine is called with the index number of the sound we wish to play.
 * The play_tune() routine is called with the index number of the tune we wish to play.
 *
 */
