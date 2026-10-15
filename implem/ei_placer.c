#include <stdlib.h>
#include "ei_placer.h"
#include "ei_implementation.h"
#include "ei_geometrymanager.h"
#include "ei_utils.h"
#include "ei_geometrymanager_prime.h"


void ei_place(ei_widget_t  widget,
              ei_anchor_t* anchor,
              int*         x,
              int*         y,
              int*         width,
              int*         height,
              float*       rel_x,
              float*       rel_y,
              float*       rel_width,
              float*       rel_height)
{
    ei_impl_geom_param_t* parameters =(ei_impl_geom_param_t*)ei_widget_get_geom_params(widget); //récupère les paramètres du widget

    if (parameters == NULL) {
        // Sans paramètres donnés, on les met par défaut
        parameters = calloc(1, sizeof(ei_impl_geom_param_t));
        parameters->anchor = ei_anc_northwest;  // On le met en nw comme précisé dan sla documentation
        parameters->x = 0;
        parameters->y = 0;
        parameters->width = 0;
        parameters->height = 0;
        parameters->rel_x = 0.0f;  // On travaille avec des float et non des double (0.0)
        parameters->rel_y = 0.0f;
        parameters->rel_width = 0.0f;
        parameters->rel_height = 0.0f;

        ei_geometrymanager_name_t name = "placer";
        ei_geometrymanager_t* placer = ei_geometrymanager_from_name(name);
        parameters->manager = placer;
        ei_widget_set_geom_params(widget, (ei_geom_param_t)parameters);
    }

    // On actualise les paramètres donnés (on ne modifie pas les NULL)
    if (anchor)
    {
        parameters->anchor = *anchor;
    }
    if (x)
    {
        parameters->x = *x;
    }
    if (y)
    {
        parameters->y = *y;
    }
    if (width)
    {
        parameters->width = *width;
    }
    if (height)
    {
        parameters->height = *height;
    }
    if (rel_x)
    {
        parameters->rel_x = *rel_x;
    }
    if (rel_y)
    {
        parameters->rel_y = *rel_y;
    }
    if (rel_width)
    {
        parameters->rel_width = *rel_width;
    }
    if (rel_height)
    {
        parameters->rel_height = *rel_height;
    }
    ei_placer_runfunc(widget);
}

