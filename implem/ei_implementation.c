// //
// // Created by walid on 4/28/26.
// //

#include "ei_implementation.h"
#include "intersection.h"
#include "widget_toplevel_class.h"



uint32_t ei_impl_map_rgba(ei_surface_t surface, ei_color_t color)
{
    int ir, ig, ib, ia;
    uint32_t pixel = 0;
    unsigned char* p = (unsigned char*)&pixel;

    hw_surface_get_channel_indices(surface, &ir, &ig, &ib, &ia);
    p[ir] = color.red;
    p[ig] = color.green;
    p[ib] = color.blue;
    if (ia >= 0) p[ia] = color.alpha;
    return pixel;
}

void ei_impl_widget_draw_children(ei_widget_t widget, ei_surface_t surface,
                                  ei_surface_t pick_surface, ei_rect_t* clipper)
{
    ei_widget_t child = widget->children_head;  //récupère la liste chainée des enfants
    while (child != NULL) {
        if (child->geom_params != NULL && child->wclass != NULL && child->wclass->drawfunc != NULL) {
            ei_rect_t* reference_rect = widget->content_rect;
            ei_rect_t parent_clip;
            bool has_parent_clip = false;

            // ancienne version avec bug :
            // pareil que dans la runfunc, si c'est une toplevel on prend la screen_location
            // if (strcmp(widget->wclass->name, "toplevel") == 0) {
            //     reference_rect = &widget->screen_location;
            // }

            // Si on a un bouton pour fermer ou pour resize, on le dessine dans toute la fenêtre et pas juste dans le content rect
            if (toplevel_is_decoration_widget(widget, child)) {
                reference_rect = &widget->screen_location;
            }

            if (clipper != NULL) {
                parent_clip = ei_rect_intersection(clipper, reference_rect);
                if (parent_clip.size.width > 0 && parent_clip.size.height > 0)
                    has_parent_clip = true;
            } else if (reference_rect != NULL) {
                parent_clip = *reference_rect;
                has_parent_clip = true;
            }

            if (has_parent_clip) {
                ei_rect_t child_clip = ei_rect_intersection(&parent_clip, &child->screen_location);
                if (child_clip.size.width > 0 && child_clip.size.height > 0)
                    child->wclass->drawfunc(child, surface, pick_surface, &child_clip);
            } else {
                child->wclass->drawfunc(child, surface, pick_surface, NULL);
            }
        }
        child = child->next_sibling;
    }
}
