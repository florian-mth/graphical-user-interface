//
// Created by florian on 28/04/2026.
//

#ifndef PROJETC_IG_WIDGET_FRAME_CLASS_H
#define PROJETC_IG_WIDGET_FRAME_CLASS_H

#include "ei_types.h"
#include "ei_implementation.h"

ei_widget_t frame_allocfunc(void);

void frame_releasefunc(ei_widget_t widget);

void frame_setdefaultsfunc(ei_widget_t widget);

void frame_draw_func(ei_widget_t widget,
                     ei_surface_t surface,
                     ei_surface_t pick_surface,
                     ei_rect_t* clipper);

typedef struct ei_frame_t {
    ei_impl_widget_t widget;
    ei_color_t bg_color;
    int border_width;
    ei_relief_t relief;

    ei_string_t text;
    ei_font_t text_font;
    ei_color_t text_color;
    ei_anchor_t text_anchor;

    ei_surface_t img;
    ei_rect_ptr_t img_rect;
    ei_anchor_t img_anchor;

    ei_rect_t inner_rect; // utile pour que les enfants puissent savoir où se placer(screen_location - borderwidth)
} ei_frame_t;

ei_widgetclass_t* frame_widgetclass(void);
void frame_geomnotifyfunc(ei_widget_t widget) ;
#endif //PROJETC_IG_WIDGET_FRAME_CLASS_H
