#ifndef VOR_COMMON_H
#define VOR_COMMON_H

#include <stdbool.h>
#include <stddef.h>

#define NONE (~0)

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
