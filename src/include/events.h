#ifndef EVENTS_H_
#define EVENTS_H_


#include"input.h"
#include"types.h"

#define FRAME_TIMER_EVENT  0

bool events_is_frame_ready(void);
bool events_is_display_closed(void);

struct all_inputs events_get_inputs(void);

bool events_prepare(struct input_bindings *input_bindings);
void events_destroy(void);


#endif /* EVENTS_H_ */
