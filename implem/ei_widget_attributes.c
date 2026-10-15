#include "ei_widget_attributes.h"
#include "ei_implementation.h"
#include "ei_application.h"
#include "ei_geometrymanager.h"

ei_widgetclass_t* ei_widget_get_class (ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    return widget->wclass;
}

const ei_color_t* ei_widget_get_pick_color (ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    return &widget->pick_color;
}

ei_widget_t ei_widget_get_parent (ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    return widget->parent;
}

ei_widget_t ei_widget_get_first_child (ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    return widget->children_head;
}

ei_widget_t ei_widget_get_last_child (ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    return widget->children_tail;
}

ei_widget_t ei_widget_get_next_sibling (ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    // On suppose par sécurité que si widget est le dernier enfant, widget->next_sibling n'est pas bien référencé
    if (((widget->parent)->children_tail)== widget) return NULL;
    return widget->next_sibling;
}

void* ei_widget_get_user_data(ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    ei_impl_widget_t* w = (ei_impl_widget_t*) widget;
    return w->user_data;
}


const ei_size_t* ei_widget_get_requested_size (ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    return &widget->requested_size;
}

void ei_widget_set_requested_size (ei_widget_t widget, ei_size_t requested_size)
{
    if (widget == NULL) return;
    bool requested_size_changed = (widget->requested_size.width != requested_size.width) ||
                                  (widget->requested_size.height != requested_size.height);
    widget->requested_size = requested_size;

    if (widget->geom_params != NULL) {
        bool relayout_done = false;
        if (requested_size_changed) {
            ei_geometrymanager_t* manager = ei_widget_get_geom_manager(widget);
            if (manager != NULL && manager->runfunc != NULL) {
                manager->runfunc(widget);
                relayout_done = true;
            }
        }
        if (!relayout_done)
            ei_app_invalidate_rect(&widget->screen_location);
    }
}

const ei_rect_t* ei_widget_get_screen_location (ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    return &widget->screen_location;
}

const ei_rect_t* ei_widget_get_content_rect	(ei_widget_t widget)
{
    if (widget == NULL) return NULL;
    return widget->content_rect;
}

void ei_widget_set_content_rect	(ei_widget_t widget, const ei_rect_t* content_rect)
{
    if (widget == NULL) return;
    if (content_rect == NULL) {
        widget->content_rect = &widget->screen_location;
        return;
    }
    widget->content_rect_storage = *content_rect;
    widget->content_rect = &widget->content_rect_storage;
}
