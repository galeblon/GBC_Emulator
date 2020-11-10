#ifndef DISPLAY_H_
#define DISPLAY_H_

#include"types.h"


#define SCALING_FACTOR 1
#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 144

typedef struct colour {
	d8   r;
	d8   g;
	d8   b;
	bool a;
} colour;


void display_prepare(float frequency, char * rom_title, bool fullscreen);
void display_draw(colour screen[SCREEN_HEIGHT][SCREEN_WIDTH]);
bool display_get_closed_status(void);
void display_destroy(void);

#endif /* DISPLAY_H_ */
