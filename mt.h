#ifndef VOR_MT_H
#define VOR_MT_H

#include <inttypes.h>

void init_mt(uint32_t s);
uint32_t urnd(void); // [0, 0xfffffff]
float frnd(void);  // [0, 1)
float crnd(void);  // [-0.5, 0.5)

#endif // VOR_MT_H
