#ifndef DISPLAY_H_
#define DISPLAY_H_

#include"types.h"

#define SCALING_FACTOR 3


typedef struct colour {
	d8   r;
	d8   g;
	d8   b;
	bool a;
} colour;


void display_prepare(float frequency, char * rom_title);
void display_draw_line(colour line[160], int index);
bool display_get_closed_status(void);
void display_destroy(void);

#endif /* DISPLAY_H_ */
