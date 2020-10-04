#include<allegro5/allegro5.h>
#ifdef OPENGL
	#include<allegro5/allegro_opengl.h>
#endif
#include"cpu.h"
#include"debug.h"
#include"display.h"
#include"logger.h"

#include "errno.h"
#include<stdio.h>


static ALLEGRO_BITMAP      *g_bitmap            = NULL;
static ALLEGRO_DISPLAY     *g_display           = NULL;
static ALLEGRO_TIMER       *g_timer             = NULL;
static ALLEGRO_EVENT_QUEUE *g_close_event_queue = NULL;
static ALLEGRO_EVENT_QUEUE *g_event_queue       = NULL;
static ALLEGRO_SHADER      *g_shader            = NULL;
static ALLEGRO_EVENT       g_event;


static const char * g_vertex_shader_source;
static const char * g_fragment_shader_source;

static const char * g_vertex_source =
	"attribute vec4 al_pos;\n"
	"attribute vec4 al_color;\n"
	"attribute vec2 al_texcoord;\n"
	"uniform mat4 al_projview_matrix;\n"
	"void main() {\n"
	"	gl_Position = al_projview_matrix * al_pos;\n"
	"}\n"
;

static const char * g_fragment_source =
	"uniform sampler2D al_tex;\n"
	"uniform ivec4 line[160];\n"
	"uniform int  scale;\n"
	"void main() {\n"
	"	vec4 colour;\n"
	"	int pixel_n;"
	"   pixel_n = int(gl_FragCoord.x)/scale;\n"
	"	colour.x = float(line[pixel_n].x)/255.0;\n"
	"	colour.y = float(line[pixel_n].y)/255.0;\n"
	"	colour.z = float(line[pixel_n].z)/255.0;\n"
	"	colour.w = float(line[pixel_n].w)/255.0;\n"
//	"	gl_FragColor = colour;\n"
	"   gl_FragColor = vec4(0.7, 0.2, 0.0, 0.5);"
	"}\n"
;


static void _display_error(enum logger_log_type type, char *title, char *message)
{
	logger_log(
		type,
		title,
		"[DISPLAY MODULE] %s\n",
		message
	);
}

static void _display_choose_shader_source(
	ALLEGRO_SHADER * shader,
	char const ** vsource,
	char const ** fsource
)
{
	char tmp1[PATH_MAX], tmp2[PATH_MAX];
	printf("? %s\n", __FILE__);
	strcpy(tmp1, __FILE__);
	printf("! %s\n", tmp1);
	strcpy(strrchr(tmp1, '/'), "/\0");
	printf("! %s\n", tmp1);
	strcpy(tmp2, tmp1);
	printf("! %s\n", tmp2);
	strcat(tmp1, "shaders/vertex_shader.glsl");
	printf("! %s\n", tmp1);
	strcat(tmp2, "shaders/fragment_shader.glsl");
	printf("! %s\n", tmp2);

	printf("! %s\n", tmp1);
	printf("! %s\n", tmp2);

	ALLEGRO_SHADER_PLATFORM platform = al_get_shader_platform(shader);
	if(platform == ALLEGRO_SHADER_HLSL) {
		//TODO?
	} else if (platform == ALLEGRO_SHADER_GLSL) {
		strcpy(*vsource, tmp1);
		strcpy(*fsource, tmp2);
		//*vsource = "shaders/vertex_shader.glsl";
		//*fsource = "shaders/fragment_shader.glsl";
	} else {
		*vsource = NULL;
		*fsource = NULL;
	}
}

void display_prepare(float frequency, char * rom_title)
{
	if(!al_init()) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO INIT",
			"FAILED TO INITIALISE ALLEGRO."
		);
		return;
	}

	g_timer = al_create_timer(frequency);
	if (!g_timer) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO TIMER",
			"FAILED TO CREATE A TIMER."
		);
		return;
	}

#ifdef OPENGL
	al_set_new_display_flags(ALLEGRO_OPENGL);
#endif
	g_display = al_create_display( 160 * SCALING_FACTOR, 144 * SCALING_FACTOR );
	if(!g_display) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO DISPLAY",
			"FAILED TO INITIALISE DISPLAY."
		);
    	return;
	}

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ABGR_8888);
	g_bitmap = al_create_bitmap( 160 * SCALING_FACTOR, 144 * SCALING_FACTOR );
	if(!g_bitmap) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO DISPLAY",
			"FAILED TO INITIALISE BITMAP."
		);
    	return;
	}

	g_event_queue = al_create_event_queue();
	if (!g_event_queue) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO QUEUE",
			"FAILED TO CREATE EVENT QUEUE."
		);
		return;
	}

	g_close_event_queue = al_create_event_queue();
	if (!g_close_event_queue) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO QUEUE",
			"FAILED TO CREATE EVENT QUEUE."
		);
		return;
	}

	//Shader stuff
	g_shader = al_create_shader(ALLEGRO_SHADER_AUTO);
	if (!g_shader) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO SHADER",
			"FAILED TO CREATE ALLEGRO SHADER."
		);
		return;
	}

	if (!al_attach_shader_source(
			g_shader,
			ALLEGRO_VERTEX_SHADER,
			g_vertex_source
		)
	) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO V SHADER",
			al_get_shader_log(g_shader)
		);
		return;
	}

	if (!al_attach_shader_source(
			g_shader,
			ALLEGRO_PIXEL_SHADER,
			g_fragment_source
		)
	) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO F SHADER",
			al_get_shader_log(g_shader)
		);
		return;
	}

	if (!al_build_shader(g_shader)) {
		_display_error(
			LOG_FATAL,
			"ALLEGRO SHADER",
			al_get_shader_log(g_shader)
		);
		return;
	}

	al_use_shader(g_shader);

	//Event registration
	al_register_event_source(
		g_close_event_queue,
		al_get_display_event_source(g_display)
	);
	al_register_event_source(
		g_event_queue,
		al_get_timer_event_source(g_timer)
	);

	al_start_timer(g_timer);

	// Display a black screen, set the title
	al_set_window_title(g_display, rom_title);
	al_clear_to_color( al_map_rgb(0, 0, 0) );
	al_flip_display();
}


void display_draw_line(colour line[160], int index)
{
	debug_assert(index < 144, "display_draw_line: index out of bounds");

#ifndef OPENGL
	al_set_target_bitmap(g_bitmap);
	ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap_region(
		g_bitmap,
		0,
		index*SCALING_FACTOR,
		160*SCALING_FACTOR,
		SCALING_FACTOR,
		ALLEGRO_PIXEL_FORMAT_ANY,
		ALLEGRO_LOCK_WRITEONLY
	);
	u8 * ptr;

	for(int y=0; y<SCALING_FACTOR; y++)
	{
		ptr = (u8*) lr->data + y * lr->pitch;
		for(int i=0; i<160; i++)
		{
			for(int j=0; j<SCALING_FACTOR; j++)
			{
				*ptr++ = line[i].r;
				*ptr++ = line[i].g;
				*ptr++ = line[i].b;
				*ptr++ = (line[i].a) ? 0 : 255;
			}
		}
	}
	al_unlock_bitmap(g_bitmap);
#endif
#ifdef OPENGL
	al_set_shader_int("scale", SCALING_FACTOR);
	int vec4[4] = {0};
	char buf[10];
	for(int i=0; i<160; i++) {
		vec4[0] = line[i].r;
		vec4[1] = line[i].g;
		vec4[2] = line[i].b;
		vec4[3] = (line[i].a) ? 0 : 255;
		sprintf(buf, "line[%d]", i);
		_display_error(
						LOG_FATAL,
						"ALLEGRO SHADER",
						buf
					);
		if(!al_set_shader_int_vector(
				buf,
				4,
				(int*)vec4,
				1
		)) {
			_display_error(
				LOG_FATAL,
				"ALLEGRO SHADER",
				"SET_SHADER_INT_VECTOR FAILED"
			);
			sleep(10);
			return;
		}
	}
	//al_set_shader_int_vector("line", 4, ?, 160);

	if(index == 143) {
		//al_set_target_backbuffer(g_display);
		//al_draw_bitmap(g_bitmap, 0, 0, 0);
		al_flip_display();
	}
#endif
}


bool display_get_closed_status(void)
{
	// Fetch the event (if one exists)
	bool event_exists = al_get_next_event(g_close_event_queue, &g_event);

	if(event_exists)
		// Handle the event
		switch (g_event.type) {
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
	al_use_shader(NULL);
	al_destroy_shader(g_shader);
	al_destroy_display(g_display);
	al_destroy_event_queue(g_event_queue);
	al_destroy_event_queue(g_close_event_queue);
}
