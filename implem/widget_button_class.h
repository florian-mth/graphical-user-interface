//
// Created by walid on 4/30/26.
//

#ifndef PROJETC_IG_WIDGET_BUTTON_CLASS_H
#define PROJETC_IG_WIDGET_BUTTON_CLASS_H

#include "ei_types.h"
#include "ei_draw.h"
#include "ei_implementation.h"
#include "ei_widget.h"
#include "ei_event.h"

typedef struct ei_polygone_t
{
    ei_point_t* tab;
    int tab_size;
}ei_polygone_t;

typedef enum button_part_t
{
    top_only,
    bottom_only,
    whole,
}button_part_t;

ei_point_t* arc(ei_point_t center, int radius, float angle_start, float angle_end, int* num_pts);
ei_polygone_t* rounded_frame(ei_rect_t rect, int radius, button_part_t part, int bw);
void draw_button(ei_surface_t surface,ei_color_t color,ei_rect_t rect,int border_width,int corner_radius,ei_relief_t relief,ei_string_t text,ei_font_t font,ei_color_t text_color,ei_anchor_t text_anchor, ei_rect_t* clipper);

typedef struct ei_button_t {
    ei_impl_widget_t widget;
    ei_color_t bg_color;
    int border_width;
    int corner_radius;
    ei_relief_t relief;

    ei_string_t text;
    ei_font_t text_font;
    ei_color_t text_color;
    ei_anchor_t text_anchor;

    ei_surface_t img;
    ei_rect_ptr_t img_rect;
    ei_anchor_t img_anchor;

    ei_callback_t callback;
    ei_user_param_t user_param;
    bool pressed;
} ei_button_t;

ei_widget_t button_allocfunc(void);
void button_releasefunc(ei_widget_t widget);
void button_setdefaultsfunc(ei_widget_t widget);
void button_drawfunc(ei_widget_t widget, ei_surface_t surface, ei_surface_t pick_surface, ei_rect_t* clipper);
void button_geomnotifyfunc(ei_widget_t widget);
ei_widgetclass_t* button_widgetclass(void);

bool button_on_press(ei_widget_t widget, ei_event_t* event, void* user_param);
bool button_on_release(ei_widget_t widget, ei_event_t* event, void* user_param);
bool button_on_move(ei_widget_t widget, ei_event_t* event, void* user_param);

#endif //PROJETC_IG_WIDGET_BUTTON_CLASS_H
