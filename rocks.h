#include "sprite.h"

int init_rocks(void);
void reset_rocks(void);

void new_rocks(void);
void move_rocks(void);
void draw_rocks(void);

int hit_rocks(float x, float y, struct shape *shape);
int pixel_hit_rocks(float x, float y);

void blast_rocks(float x, float y, float radius, int onlyslow);
