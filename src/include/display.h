#ifndef DISPLAY_H_
#define DISPLAY_H_

#include"types.h"

#define SCALING_FACTOR 1

void display_prepare(float frequency, char * rom_title);
bool display_get_closed_status(void);
void display_destroy(void);

#endif /* DISPLAY_H_ */
