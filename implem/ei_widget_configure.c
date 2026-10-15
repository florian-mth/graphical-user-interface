#include "ei_widget_configure.h"
#include "ei_implementation.h"
#include "ei_application.h"
#include "hw_interface.h"
#include "ei_types.h"
#include "ei_types.h"
#include "ei_event.h"
#include "ei_utils.h"
#include "hw_interface.h"
#include "ei_geometrymanager.h"
#include "ei_widget_attributes.h"
#include "widget_frame_class.h"
#include "widget_button_class.h"
#include "widget_toplevel_class.h"

static char* ei_duplicate_string(const char* src)
{
    if (src == NULL)
        return NULL;

    size_t len = strlen(src) + 1;
    char* copy = malloc(len);
    if (copy == NULL)
        return NULL;

    memcpy(copy, src, len);
    return copy;
}

void ei_frame_configure	(ei_widget_t		widget,
                             ei_size_t*		requested_size,
                             const ei_color_t*	color,
                             int*			border_width,
                             ei_relief_t*		relief,
                             ei_string_t*		text,
                             ei_font_t*		text_font,
                             ei_color_t*		text_color,
                             ei_anchor_t*		text_anchor,
                             ei_surface_t*		img,
                             ei_rect_ptr_t*		img_rect,
                             ei_anchor_t*		img_anchor)
{
    ei_frame_t* frame = (ei_frame_t*)widget;

    if (requested_size) {
        ei_widget_set_requested_size(widget, *requested_size);
    }
    if (color) frame->bg_color = *color;
    if (border_width) frame->border_width = *border_width;
    if (relief) frame->relief = *relief;
    // permet d'éviter les leaks de mémoire, car juste avec le pointeur on peut la libérer et dans ce cas on pointe vers une case mémoire "vide"
    // mais si on le duplique même si l'original est free on aura pas de Segfault
    if (text) {
        if (frame->text != NULL) free(frame->text);
        frame->text = ei_duplicate_string(*text);
    }
    if (requested_size == NULL && frame->text != NULL) {
        // Si on ne définit pas la taille
        if (widget->requested_size.width == 0 && widget->requested_size.height == 0) {
            int text_width, text_height;
            hw_text_compute_size(frame->text, frame->text_font, &text_width, &text_height);
            ei_widget_set_requested_size(widget, (ei_size_t){text_width, text_height});
        }
    }

    if (text_font) frame->text_font = *text_font;
    if (text_color) frame->text_color = *text_color;
    if (text_anchor) frame->text_anchor = *text_anchor;
    if (img) frame->img = *img;
    if (img_rect) frame->img_rect = *img_rect;
    if (img_anchor) frame->img_anchor = *img_anchor;
    if (widget->geom_params != NULL) {
        // On appelle runfunc que si requested_size a changé
        if (requested_size != NULL) {
            if (widget->geom_params->manager != NULL &&
                widget->geom_params->manager->runfunc != NULL)
                widget->geom_params->manager->runfunc(widget);
        }
        ei_app_invalidate_rect(&widget->screen_location);
    }
}

void ei_button_configure (ei_widget_t		widget,
                             ei_size_t*		requested_size,
                             const ei_color_t*	color,
                             int*			border_width,
                             int*			corner_radius,
                             ei_relief_t*		relief,
                             ei_string_t*		text,
                             ei_font_t*		text_font,
                             ei_color_t*		text_color,
                             ei_anchor_t*		text_anchor,
                             ei_surface_t*		img,
                             ei_rect_ptr_t*		img_rect,
                             ei_anchor_t*		img_anchor,
                             ei_callback_t*		callback,
                             ei_user_param_t*	user_param) {
    ei_button_t* button = (ei_button_t*)widget;
    const int horizontal_padding = 20;
    const int vertical_padding = 10;

    if (requested_size) widget->requested_size = *requested_size;
    if (color) button->bg_color = *color;
    if (border_width) button->border_width = *border_width;
    if (corner_radius) button->corner_radius = *corner_radius;
    if (relief) button->relief = *relief;
    if (text) {
        // permet d'éviter les leaks de mémoire, car juste avec le pointeur on peut la libérer et dans ce cas on pointe vers une case mémoire "vide"
        // mais si on le duplique même si l'original est free on aura pas de Segfault
        if (button->text != NULL) free(button->text);
        button->text = ei_duplicate_string(*text);
    }
    if (text_font) button->text_font = *text_font;
    if (text_color) button->text_color = *text_color;
    if (text_anchor) button->text_anchor = *text_anchor;
    if (img) {
        if (button->img != NULL) {
            hw_surface_free(button->img);
            button->img = NULL;
        }
        button->img_rect = NULL;

        if (*img != NULL) {
            // fait une copie propre de l'image car : Pour le jeu de puzzle on copie l'adresse d'une image pour faire notre dessin
            // or cette image est libérée à la fin de create_puzzle_window donc quand on dessine on essaye de dessiner quelque chose
            // qui a ete liberée donc ça crash. Donc on doit copier et stocker proprement la surface image pour ensuite la dessiner
            ei_rect_t src_rect = img_rect ? **img_rect : hw_surface_get_rect(*img);
            button->img = hw_surface_create(*img, src_rect.size, false);
            hw_surface_lock(*img);
            hw_surface_lock(button->img);
            ei_copy_surface(button->img, NULL, *img, &src_rect, false);
            hw_surface_unlock(button->img);
            hw_surface_unlock(*img);
            // img_rect devient NULL car on a déjà extrait la bonne région
            button->img_rect = NULL;
        }
    }
    if (img_rect) button->img_rect = *img_rect;
    if (img_anchor) button->img_anchor = *img_anchor;
    if (callback) button->callback = *callback;
    if (user_param) button->user_param = *user_param;

    if (requested_size == NULL) {
        int width = widget->requested_size.width;
        int height = widget->requested_size.height;

        if (button->text != NULL) {
            int text_width = 0;
            int text_height = 0;
            hw_text_compute_size(button->text, button->text_font, &text_width, &text_height);

            if (width == 0) {
                width = text_width + 2 * (button->border_width + horizontal_padding);
            }
            if (height == 0) {
                height = text_height + 2 * (button->border_width + vertical_padding);
            }
        }

        widget->requested_size = (ei_size_t){width, height};
    }
}

void ei_toplevel_configure(ei_widget_t    widget,
                           ei_size_t*     requested_size,
                           const ei_color_t* color,
                           int*           border_width,
                           ei_string_t*   title,
                           bool*          closable,
                           ei_axis_set_t* resizable,
                           ei_size_ptr_t* min_size)
{
    ei_toplevel_t* toplevel = (ei_toplevel_t*)widget;
    bool requested_size_dirty = false;
    bool needs_redraw = false;

    if (requested_size) {
        toplevel->requested_size = *requested_size;
        requested_size_dirty = true;
    }
    if (color) {
        toplevel->bg_color = *color;
        needs_redraw = true;
    }
    if (border_width) {
        toplevel->border_width = *border_width;
        requested_size_dirty = true;
    }
    if (title) {
        if (toplevel->title != NULL)
            free(toplevel->title);
        toplevel->title = ei_duplicate_string(*title);
        needs_redraw = true;
    }

    if (closable)
    {
        toplevel->closable = *closable;
        if (*closable == false)
        {
            ei_widget_destroy(toplevel->close_button);
            toplevel->close_button = NULL; // remis à null pour éviter les bug (case qui se remonte 2048)
        }
    }


    if (resizable)
    {
        toplevel->resizable = *resizable;
        if (*resizable == ei_axis_none)
        {
            ei_widget_destroy(toplevel->size_button);
            toplevel->size_button = NULL; // remis à null pour éviter les bug (case qui se remonte 2048)
        }
    }

    if (min_size) {
        if (*min_size != NULL)
            toplevel->min_size = **min_size;
    }

    if (requested_size_dirty) {

        ei_size_t outer_size = {
            toplevel->requested_size.width  + 2 * toplevel->border_width,
            toplevel->requested_size.height + toplevel->header_height + toplevel->border_width
        };

        ei_widget_set_requested_size(widget, outer_size);

        // Relance le geometry manager si le widget est déjà placé.
        if (widget->geom_params != NULL &&
            widget->geom_params->manager != NULL &&
            widget->geom_params->manager->runfunc != NULL)
        {
            widget->geom_params->manager->runfunc(widget);
        }

        ei_app_invalidate_rect(&widget->screen_location);
    }


}
