#ifndef VOR_MT_H
#define VOR_MT_H

#include <stdint.h>

void init_mt(uint32_t s);
uint32_t irnd(void); // [0, 0xfffffff]
float frnd(void);  // [0, 1)
float crnd(void);  // [-0.5, 0.5)

#endif // VOR_MT_H
