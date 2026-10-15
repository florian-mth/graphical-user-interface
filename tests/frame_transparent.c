//
// Created by walid on 5/15/26.
//
#include <stdio.h>
#include <stdlib.h>

#include "ei_application.h"
#include "ei_event.h"
#include "hw_interface.h"
#include "ei_widget_configure.h"
#include "ei_placer.h"


bool default_handler(ei_widget_t widget, ei_event_t* event, ei_user_param_t user_param)
{
	if ( (event->type == ei_ev_close) ||
		((event->type == ei_ev_keydown) && (event->param.key_code == SDLK_ESCAPE))) {
		ei_app_quit_request();
		return true;
		} else
			return false;
}

int main(int argc, char** argv)
{
	ei_widget_t	frame;
	ei_widget_t	frame1;
	ei_widget_t	frame2;
	ei_widget_t	frame3;
	ei_widget_t	frame4;
	ei_widget_t	frame5;
	ei_widget_t	frame6;

	/* Create the application and change the color of the background. */
	ei_app_create((ei_size_t){600, 600}, false);
	ei_frame_set_bg_color(ei_app_root_widget(), (ei_color_t){0x52, 0x7f, 0xb4, 0xff});

	frame6 = ei_widget_create	("frame", ei_app_root_widget(), NULL, NULL);
	ei_frame_configure		(frame6, &(ei_size_t){100,100},
						   &(ei_color_t){0x88, 0x88, 0x88, 0x88},
						 &(int){6},
						 &(ei_relief_t){ei_relief_raised}, &(ei_string_t){" moins"}, NULL, NULL, NULL, NULL, NULL, NULL);
	ei_place_xy			(frame6, 300, 150);
	/* Create, configure and place the frame on screen. */
	frame = ei_widget_create	("frame", ei_app_root_widget(), NULL, NULL);
	ei_frame_configure		(frame, &(ei_size_t){300,200},
						   &(ei_color_t){0x88, 0x88, 0x88, 0x20},
						 &(int){6},
						 &(ei_relief_t){ei_relief_raised}, &(ei_string_t){"je transparais"}, NULL, NULL, NULL, NULL, NULL, NULL);
	ei_place_xy			(frame, 150, 200);
	/* Create, configure and place the frame on screen. */
	frame1 = ei_widget_create	("frame", frame, NULL, NULL);
	ei_frame_configure		(frame1, &(ei_size_t){70,40},
						   &(ei_color_t){0x88, 0x88, 0x88, 0xff},
						 &(int){6},
						 &(ei_relief_t){ei_relief_raised}, &(ei_string_t){"1 ?!"}, NULL, NULL, NULL, NULL, NULL, NULL);
	ei_place_xy			(frame1, 0, 0);
	/* Create, configure and place the frame on screen. */
	frame2 = ei_widget_create	("frame", frame, NULL, NULL);
	ei_frame_configure		(frame2, &(ei_size_t){70,40},
						   &(ei_color_t){0x88, 0x88, 0x88, 0xaa},
						 &(int){6},
						 &(ei_relief_t){ei_relief_raised}, &(ei_string_t){"2 ?!"}, NULL, NULL, NULL, NULL, NULL, NULL);
	ei_place_xy			(frame2, 0, 150);
	/* Create, configure and place the frame on screen. */
	frame3 = ei_widget_create	("frame", frame, NULL, NULL);
	ei_frame_configure		(frame3, &(ei_size_t){70,40},
						   &(ei_color_t){0x88, 0x88, 0x88, 0x55},
						 &(int){6},
						 &(ei_relief_t){ei_relief_raised}, &(ei_string_t){"3 ?!"}, NULL, NULL, NULL, NULL, NULL, NULL);
	ei_place_xy			(frame3, 220, 0);
	/* Create, configure and place the frame on screen. */
	frame4 = ei_widget_create	("frame", frame, NULL, NULL);
	ei_frame_configure		(frame4, &(ei_size_t){70,40},
						   &(ei_color_t){0x88, 0x88, 0x88, 0x00},
						 &(int){6},
						 &(ei_relief_t){ei_relief_raised}, &(ei_string_t){"4 ?!"}, NULL, NULL, NULL, NULL, NULL, NULL);
	ei_place_xy			(frame4, 220, 150);
	frame5 = ei_widget_create	("frame", ei_app_root_widget(), NULL, NULL);
	ei_frame_configure		(frame5, &(ei_size_t){100,100},
						   &(ei_color_t){0x88, 0x88, 0x88, 0x88},
						 &(int){6},
						 &(ei_relief_t){ei_relief_raised}, &(ei_string_t){"5 ?!"}, NULL, NULL, NULL, NULL, NULL, NULL);
	ei_place_xy			(frame5, 100, 150);

	ei_bind(ei_ev_keydown,		NULL, "all", default_handler, NULL);
	ei_bind(ei_ev_close,		NULL, "all", default_handler, NULL);

	/* Run the application's main loop. */
	ei_app_run();

	ei_unbind(ei_ev_keydown,	NULL, "all", default_handler, NULL);
	ei_unbind(ei_ev_close,		NULL, "all", default_handler, NULL);
	/* We just exited from the main loop. Terminate the application (cleanup). */
	ei_app_free();

	return (EXIT_SUCCESS);
}
