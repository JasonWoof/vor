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

static inline float
fconstrain(float f, float max)
{
	max -= SMIDGE;

	if(f > max) {
		return max;
	}
	if(f < SMIDGE) {
		return SMIDGE;
	}

	return f;
}

static inline float
fconstrain2(float f, float min, float max)
{
	min += SMIDGE;
	max -= SMIDGE;

	if(f > max) {
		return max;
	}
	if(f < min) {
		return min;
	}

	return f;
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
