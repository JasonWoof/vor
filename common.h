#ifndef VOR_COMMON_H
#define VOR_COMMON_H

#ifndef NULL
#define NULL 0
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef abs
#define abs(a) ((a)<=0 ? -(a) : (a))
#endif

#define CONDERROR(a) if((a)) {initerror = strdup(SDL_GetError());return 1;}
#define NULLERROR(a) CONDERROR((a) == NULL)

#endif // VOR_COMMON_H
