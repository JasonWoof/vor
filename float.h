#ifndef VOR_FLOAT_H
#define VOR_FLOAT_H

#include <math.h>

static inline int
fclip(float f, float max)
{
	return f < 0 || (float)f >= (float)max;
}

static inline float
fwrap(float f, float max)
{
	if((float)f >= (float)max) f = (float)f - (float)max;
	else if(f < 0) {
		f += max;
		if((float)f >= (float)max) f = nextafterf(f, 0);
	}
	return f;
}

#endif // VOR_FLOAT_H
