#ifndef DISPLAY_H_
#define DISPLAY_H_

#include"cpu.h"

#define SCALING_FACTOR 1

void display_prepare(float frequency);
void display_create_window(char * rom_title);
bool display_get_closed_status(void);
void display_destroy(void);

#endif /* DISPLAY_H_ */
