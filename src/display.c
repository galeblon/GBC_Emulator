#include<allegro5/allegro5.h>
#include"display.h"
#include"logger.h"

ALLEGRO_DISPLAY 	*display 			= NULL;
ALLEGRO_TIMER 		*timer 				= NULL;
ALLEGRO_EVENT_QUEUE *close_event_queue 	= NULL;
ALLEGRO_EVENT_QUEUE *event_queue 		= NULL;
ALLEGRO_EVENT 		event;


static void _display_error(enum logger_log_type type, char *title, char *message)
{
	char *full_message = logger_get_msg_buffer();
	snprintf(
		full_message,
		LOG_MESSAGE_MAX_SIZE,
		"[DISPLAY MODULE] %s\n",
		message
	);
	logger_log(
		type,
		title,
		full_message
	);
}

void display_prepare(float frequency)
{
	if( !al_init() ) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO INIT",
			"FAILED TO INITIALISE ALLEGRO."
		);
		return;
	}

	timer = al_create_timer(frequency);
	if (!timer) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO TIMER",
			"FAILED TO CREATE A TIMER."
		);
		return;
	}

	display = al_create_display( 160 * SCALING_FACTOR, 144 * SCALING_FACTOR );
	if( !display ) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO DISPLAY",
			"FAILED TO INITIALISE DISPLAY."
		);
    	return;
	}

	event_queue = al_create_event_queue();
	if (!event_queue) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO QUEUE",
			"FAILED TO CREATE EVENT QUEUE."
		);
		return;
	}

	close_event_queue = al_create_event_queue();
	if (!close_event_queue) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO QUEUE",
			"FAILED TO CREATE EVENT QUEUE."
		);
		return;
	}

	// Register event sources
	al_register_event_source(
		close_event_queue,
		al_get_display_event_source(display)
	);
	al_register_event_source(
		event_queue,
		al_get_timer_event_source(timer)
	);

	// Start the timer
	al_start_timer(timer);
}


void display_create_window(char * rom_title)
{
	// Display a black screen, set the title
	al_set_window_title(display, rom_title);
	al_clear_to_color( al_map_rgb(0, 0, 0) );
	al_flip_display();
}


bool display_get_closed_status(void)
{
	// Fetch the event (if one exists)
	bool event_exists = al_get_next_event(close_event_queue, event);

	if(event_exists)
		// Handle the event
		switch (event.type) {
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				return true;
			default:
				return false;
		}
	else
		return false;
}


void display_destroy(void)
{
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_destroy_event_queue(close_event_queue);
}
