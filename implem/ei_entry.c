#include "ei_entry.h"
#include <string.h>
#include "ei_application.h"
#include "ei_widget_attributes.h"
#include "widget_entry_class.h"

void ei_entry_configure (ei_widget_t widget, int* requested_char_size, const ei_color_t* color,
    int* border_width, ei_font_t* text_font, ei_color_t* text_color)
{
    if (widget == NULL) return;
    ei_entry_t* entry = (ei_entry_t*)widget;

    if (color) entry->bg_color = *color;
    if (border_width) entry->border_width = (*border_width < 2) ? 2 : *border_width;
    if (text_font) entry->text_font = *text_font;
    if (text_color) entry->text_color = *text_color;
    if (requested_char_size) entry->requested_char_size = *requested_char_size;

    // Si un des params qui influence la taille change, on re-calcule la requested_size
    if (requested_char_size || border_width || text_font) {
        int tw, th;
        hw_text_compute_size("m", entry->text_font, &tw, &th);
        ei_size_t new_requested_size = (ei_size_t){
            tw * entry->requested_char_size + 2 * (entry->border_width + 6),
            th + 2 * (entry->border_width + 4)
        };
        ei_widget_set_requested_size(widget, new_requested_size);
    } else if (widget->geom_params != NULL) {
        ei_app_invalidate_rect(&widget->screen_location);
    }
}

void ei_entry_set_text (ei_widget_t widget, ei_const_string_t text)
{
    if (widget == NULL) return;
    ei_entry_t* entry = (ei_entry_t*) widget;
    if (entry->text_input != NULL) free(entry->text_input);
    if (text) entry->text_input = strdup(text);
    else
        entry->text_input = NULL;
    ei_app_invalidate_rect(&widget->screen_location);
}

ei_const_string_t ei_entry_get_text (ei_widget_t widget)
{
    if (widget==NULL) return NULL;
    ei_entry_t* entry = (ei_entry_t*) widget;
    return entry->text_input;
}

void ei_entry_give_focus(ei_widget_t widget)
{
    focus_on_entry(widget);
}
