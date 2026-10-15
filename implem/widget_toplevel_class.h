#ifndef WIDGET_TOPLEVEL_CLASS_H
#define WIDGET_TOPLEVEL_CLASS_H

#include "ei_types.h"
#include "ei_implementation.h"
#include "ei_widgetclass.h"
#include "ei_widget_configure.h"
#include "widget_button_class.h"

typedef struct ei_toplevel_t {
    ei_impl_widget_t widget;
    ei_color_t       bg_color;
    ei_color_t       header_color;
    int              header_corner_radius;
    ei_size_t        requested_size;
    ei_size_t        min_size;
    int              header_height;
    int              border_width;
    bool             closable;
    ei_axis_set_t    resizable;
    ei_string_t      title;
    ei_font_t        text_font;
    ei_color_t       text_color;
    ei_anchor_t      text_anchor;
    ei_surface_t     img;
    ei_rect_ptr_t    img_rect;
    ei_anchor_t      img_anchor;
    ei_widget_t      close_button;
    ei_widget_t      size_button;
    ei_rect_t        content_rect_storage;
} ei_toplevel_t;



ei_widget_t       toplevel_allocfunc     (void);
void              toplevel_releasefunc   (ei_widget_t widget);
void              toplevel_setdefaultsfunc(ei_widget_t widget);
void              toplevel_drawfunc      (ei_widget_t widget,
                                          ei_surface_t surface,
                                          ei_surface_t pick_surface,
                                          ei_rect_t*   clipper);
void              toplevel_geomnotifyfunc(ei_widget_t widget);
ei_widgetclass_t* toplevel_widgetclass   (void);
void              draw_toplevel          (ei_surface_t surface,
                                          int           r,
                                          ei_color_t    headercolor,
                                          ei_color_t    bg_color,
                                          ei_rect_t     rect,
                                          int           header_height,
                                          int           border_width,
                                          ei_string_t   title,
                                          ei_font_t     font,
                                          ei_color_t    title_color,
                                          ei_rect_t*    clipper);

bool toplevel_close(ei_widget_t widget, ei_event_t*, void* user_param);
bool toplevel_resize(ei_widget_t widget, ei_event_t* event, void* user_param);
bool toplevel_on_move(ei_widget_t widget, ei_event_t* event, void* user_param);
bool toplevel_on_release(ei_widget_t widget, ei_event_t* event, void* user_param);
bool toplevel_on_drag(ei_widget_t widget, ei_event_t* event, void* user_param);
bool toplevel_resize_drag(ei_widget_t widget, ei_event_t* event, void* user_param);
bool toplevel_resize_release(ei_widget_t widget, ei_event_t* event, void* user_param);
bool toplevel_is_decoration_widget(ei_widget_t toplevel_widget, ei_widget_t child);


void	ei_draw_text_symetrique	(ei_surface_t		surface,
                 const ei_point_t*	where,
                 ei_const_string_t	text,
                 ei_font_t		font,
                 ei_color_t		color,
                 const ei_rect_t*	clipper);  // définie dans ei_draw.c
ei_rect_t rect_union(ei_rect_t a, ei_rect_t b);

#endif // WIDGET_TOPLEVEL_CLASS_H
