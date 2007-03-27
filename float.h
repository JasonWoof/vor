#ifndef VOR_FLOAT_H
#define VOR_FLOAT_H

#include <math.h>

#define SMIDGE 0.0001

// return true if f is OUTSIDE the range [SMIDGE..(max-SMIDGE)]
static inline int
fclip(float f, float max)
{
	return f < SMIDGE || f >= (max - SMIDGE);
}

// wrap f so it's within the range [SMIDGE..(max-SMIDGE)]
// assumes f is not outside this range by more than (max - (2 * SMIDGE))
static inline float
fwrap(float f, float max)
{
	float upper = max - SMIDGE;
	float range = upper - SMIDGE;

	if(f > upper) {
		f -= range;
	}
	if(f < SMIDGE) {
		f += range;
	}

	return f;
}

#endif // VOR_FLOAT_H
