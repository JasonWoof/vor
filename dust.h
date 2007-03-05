#ifndef VOR_DUST_H
#define VOR_DUST_H

/*
 * Dust motes are harmless background items.  They are created when the
 * program is initialized, and are simply wrapped whenever they hit the
 * edge of the screen.
 */

#define N_DUST_MOTES 2000
#define MAX_DUST_DEPTH 2

void init_dust(void);
void move_dust(void);
void draw_dust(void);

#endif // VOR_DUST_H
