#ifndef EVENTS_H_
#define EVENTS_H_


#include"input.h"
#include"types.h"


bool events_get_closed_status(void);
bool events_get_frame_status(void);
void events_reset_frame_status(void);
void events_prepare(void);
void events_step(void (*input_function)(SDL_Event, struct all_inputs*), struct all_inputs* inputs);


#endif /* EVENTS_H_ */
