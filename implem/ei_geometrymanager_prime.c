//
// Created by ringuede on 04/05/2026.
//
#include "ei_types.h"
#include "ei_implementation.h"
#include "widget_toplevel_class.h"
// fonction qui permet de placer avec une ancre (potentiellemnt null)
ei_rect_t ei_place_anchor_in_rect(ei_rect_t container, ei_size_t size, ei_anchor_t anchor)
{
    ei_rect_t rect = {
        .top_left = container.top_left,
        .size = size
    };

    switch (anchor) {
        case ei_anc_none:
        case ei_anc_northwest:
            break;
        case ei_anc_north:
            rect.top_left.x += (container.size.width - size.width) / 2;
            break;
        case ei_anc_northeast:
            rect.top_left.x += container.size.width - size.width;
            break;
        case ei_anc_west:
            rect.top_left.y += (container.size.height - size.height) / 2;
            break;
        case ei_anc_center:
            rect.top_left.x += (container.size.width - size.width) / 2;
            rect.top_left.y += (container.size.height - size.height) / 2;
            break;
        case ei_anc_east:
            rect.top_left.x += container.size.width - size.width;
            rect.top_left.y += (container.size.height - size.height) / 2;
            break;
        case ei_anc_southwest:
            rect.top_left.y += container.size.height - size.height;
            break;
        case ei_anc_south:
            rect.top_left.x += (container.size.width - size.width) / 2;
            rect.top_left.y += container.size.height - size.height;
            break;
        case ei_anc_southeast:
            rect.top_left.x += container.size.width - size.width;
            rect.top_left.y += container.size.height - size.height;
            break;
    }

    return rect;
}

void ei_placer_runfunc (ei_widget_t widget) {
    ei_impl_widget_t* widget_extended  = (ei_impl_widget_t*)widget;
    ei_impl_geom_param_t* parameters = (ei_impl_geom_param_t*)ei_widget_get_geom_params(widget);

    ei_impl_widget_t* parent = ((ei_impl_widget_t*)widget_extended->parent);
    ei_rect_t parent_rect =*parent->content_rect;
    /* === Ancienne Version de ei_placer_runfunc avec des bugs === */
    // ei_rect_t parent_rect;
    // on fait un test ou si le parent est un toplevel et que le widget et un bouton pour fermer ou redimensionner on dis que le parent rect est la screen location
    // et non le content rect( pour pouvoir mettre le bouton dans le cadre de la fenetre)
    // if (strcmp(parent->wclass->name, "toplevel")==0 && (widget == ((ei_toplevel_t*)parent)->close_button || widget==((ei_toplevel_t*)parent)->size_button)) {
    //     ei_rect_t parent_rect =*parent->screen_location;
    //     }else {
    //         parent_rect = *parent->content_rect;
    //     }

    // Pareil que pour draw_children, si on a une decoration ou mets la screen_location
    if (toplevel_is_decoration_widget((ei_widget_t)parent, widget)) {
        parent_rect = parent->screen_location;
    }


    int x = parent_rect.top_left.x + parameters->x + (int)(parameters->rel_x * parent_rect.size.width);
    int y = parent_rect.top_left.y + parameters->y + (int)(parameters->rel_y * parent_rect.size.height);

    int width = parameters->width + (int)(parameters->rel_width  * parent_rect.size.width);
    int height = parameters->height + (int)(parameters->rel_height * parent_rect.size.height);

    // Cas où on ne donne pas de taille (donc taille = 0) par exemple pour ei_place_xy
    if (width == 0)
    {
        width = widget_extended->requested_size.width;
    }
    if (height == 0)
    {
        height = widget_extended->requested_size.height;
    }

    ei_rect_t anchor_rect = {
        .top_left = {x, y},
        .size = {0, 0}
    };
    ei_rect_t new_location = ei_place_anchor_in_rect(anchor_rect, (ei_size_t){width, height}, parameters->anchor);
    eip_geometrymanager_finalize(widget, &new_location);
}

static void placer_releasefunc(ei_widget_t widget)
{
    (void)widget;
}


//Seul geometry_manager enregistré (dans ei_app.c)
ei_geometrymanager_t* placer_widgetclass(void)
{
    static ei_geometrymanager_t g_placer_manager = {
        "placer",
        ei_placer_runfunc,
        placer_releasefunc,
        NULL
    };
    return &g_placer_manager;
}
