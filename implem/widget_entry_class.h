#ifndef PROJETC_IG_WIDGET_ENTRY_CLASS_H
#define PROJETC_IG_WIDGET_ENTRY_CLASS_H

#include "ei_types.h"
#include "ei_draw.h"
#include "ei_implementation.h"
#include "ei_widget.h"
#include "ei_event.h"

typedef struct ei_entry_t {
    ei_impl_widget_t widget;
    ei_color_t bg_color;
    int border_width;
    ei_color_t border_color;

    ei_string_t text_input;
    ei_font_t text_font;
    ei_color_t text_color;
    int requested_char_size;
    int cursor_pos;
    int scroll_offset;

    ei_callback_t callback;
    ei_user_param_t user_param;
    bool selected;
} ei_entry_t;

ei_widget_t entry_allocfunc(void);
void entry_releasefunc(ei_widget_t widget);
void entry_setdefaultsfunc(ei_widget_t widget);
void entry_drawfunc(ei_widget_t widget, ei_surface_t surface, ei_surface_t pick_surface, ei_rect_t* clipper);
void entry_geomnotifyfunc(ei_widget_t widget);
ei_widgetclass_t* entry_widgetclass(void);
void focus_on_entry(ei_widget_t widget);

bool entry_on_click     (ei_widget_t widget, ei_event_t* event, void* user_param);
bool entry_on_text_input(ei_widget_t widget, ei_event_t* event, void* user_param);
bool entry_on_keydown   (ei_widget_t widget, ei_event_t* event, void* user_param);
bool entry_blink_handler(ei_widget_t widget,ei_event_t* event,void* user_param);

#endif //PROJETC_IG_WIDGET_ENTRY_CLASS_H
