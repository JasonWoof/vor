#include <stdio.h>

#define TEMPLATE_WIDTH 1870
#define TEMPLATE_HEIGHT 17

#define OUTPUT_WIDTH 29.0

#define FACTOR (OUTPUT_WIDTH / (float)TEMPLATE_WIDTH)
#define XMIN (0 - (TEMPLATE_WIDTH / 2))
#define YMIN (TEMPLATE_HEIGHT / 2)

int main(int argc, char** argv) {
	int bit, x, y;

	x = XMIN; y = YMIN;
	while((bit = getc(stdin)) != EOF) {
		if(bit == '\n') {
			x = XMIN;
			--y;
			continue;
		}

		if(bit == '#') {
			// parameters to sphere (in a blob) are: {<x, y, z>, diameter, center-density}
			printf("sphere { <%f, %f, 0>, %f, 2 }\n", (float)(x * FACTOR), (float)(y * FACTOR), (float)(2.5 * FACTOR));
		}
		++x;
	}

	return 0;
}
