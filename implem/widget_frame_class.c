//
// Created by ringuede on 28/04/2026.
//

#include "ei_implementation.h"
#include "ei_types.h"
#include "ei_geometrymanager_prime.h"
#include "widget_frame_class.h"
#include "widget_button_class.h"
#include "intersection.h"

ei_widget_t frame_allocfunc(void) {
    ei_frame_t* frame = calloc(1, sizeof(ei_frame_t));
    return (ei_widget_t)frame;
}

void frame_releasefunc(ei_widget_t widget) {
    ei_frame_t* frame=(ei_frame_t*)widget;
    if (frame->img_rect != NULL) {
        free(frame->img_rect);
        frame->img_rect = NULL;
    }
    if (frame->text!=NULL) {
        free(frame->text);
        frame->text = NULL;
    }
}

void frame_setdefaultsfunc(ei_widget_t widget) {
    ei_frame_t* frame=(ei_frame_t*)widget;
    frame->bg_color= ei_default_background_color;
    frame->border_width=0;
    frame->relief= ei_relief_none;
    frame->text= NULL;
    frame->text_font= ei_default_font;
    frame->text_color=ei_font_default_color;
    frame->text_anchor=ei_anc_center;
    frame->img= NULL;
    frame->img_rect= NULL;
    frame->img_anchor= ei_anc_center;

    widget->requested_size = (ei_size_t){0,0};
    widget->content_rect = &widget->screen_location;

}

/* === Ancienne Version de frame_drawfunc SANS relief === */

// void frame_drawfunc(ei_widget_t widget, ei_surface_t surface, ei_surface_t pick_surface, ei_rect_t* clipper) {
//     ei_frame_t* frame=(ei_frame_t*)widget;
//     ei_fill(surface, &frame->bg_color, &widget->screen_location);
//     ei_impl_widget_draw_children(widget,surface,pick_surface,clipper); // appelle le draw pour les enfants de la frame
// }

/* === Ancienne Version de frame_drawfunc SANS relief === */

// void frame_drawfunc(ei_widget_t widget, ei_surface_t surface, ei_surface_t pick_surface, ei_rect_t* clipper) {
//     ei_frame_t* frame=(ei_frame_t*)widget;
//
//     //Pour mettre du relief
//     if (frame->relief != ei_relief_none && frame->border_width > 0) {
//         draw_button(surface,frame->bg_color,widget->screen_location,frame->border_width,0,frame->relief,NULL,NULL,frame->text_color, clipper);
//     } else {
//         ei_fill(surface, &frame->bg_color, &widget->screen_location);
//     }
//
//     ei_impl_widget_draw_children(widget,surface,pick_surface,clipper); // appelle le draw pour les enfants de la frame
// }

ei_polygone_t* rect_frame(ei_rect_t rect,  button_part_t part, int bw) {
    ei_point_t c_tl = {rect.top_left.x , rect.top_left.y };
    ei_point_t c_tr = {rect.top_left.x + rect.size.width  , rect.top_left.y };
    ei_point_t c_br = {rect.top_left.x + rect.size.width , rect.top_left.y + rect.size.height };
    ei_point_t c_bl = {rect.top_left.x , rect.top_left.y + rect.size.height  };

    int h = bw;
    ei_point_t c_tl_int = {rect.top_left.x +h , rect.top_left.y +h };
    ei_point_t c_tr_int = {rect.top_left.x + rect.size.width -h , rect.top_left.y +h };
    ei_point_t c_br_int = {rect.top_left.x + rect.size.width -h , rect.top_left.y + rect.size.height - h };
    ei_point_t c_bl_int = {rect.top_left.x +h, rect.top_left.y + rect.size.height - h };


    ei_point_t* points = malloc(200 * sizeof(ei_point_t));
    int pos = 0, n = 0;
    ei_point_t* tmp;

    if (part == whole) {
        points[pos++] = c_tl_int;
        points[pos++] = c_tr_int;
        points[pos++] = c_br_int;
        points[pos++] = c_bl_int;
    } else {

        if (part == top_only) {
            points[pos++] = c_bl;
            points[pos++] = c_tl;
            points[pos++] = c_tr;
            points[pos++] = c_tr_int;
            points[pos++] = c_tl_int;
            points[pos++] = c_bl_int;
        } else {
            points[pos++] = c_tr;
            points[pos++] = c_br;
            points[pos++] = c_bl;
            points[pos++] = c_bl_int;
            points[pos++] = c_br_int;
            points[pos++] = c_tr_int;
        }
    }

    ei_polygone_t* res = malloc(sizeof(ei_polygone_t));
    res->tab = points;
    res->tab_size = pos;
    return res;
}

void draw_frame(ei_surface_t surface, ei_color_t color, ei_rect_t rect,
                 int bw, int r, ei_relief_t relief,
                 ei_string_t text, ei_font_t font, ei_color_t text_color, ei_rect_t* clipper) {

    ei_color_t dark = {color.red/2, color.green/2, color.blue/2, color.alpha}; // Moyenne entre la couleur et le noir
    ei_color_t light = {(color.red+255)/2, (color.green+255)/2, (color.blue+255)/2, color.alpha};  // Moyenne entre la couleur et le blanc
    ei_color_t top_c = (relief == ei_relief_sunken) ? dark : light;
    ei_color_t bot_c = (relief == ei_relief_sunken) ? light : dark;

    ei_polygone_t* hi = rect_frame(rect, top_only, bw);
    ei_polygone_t* lo = rect_frame(rect, bottom_only, bw);
    ei_draw_polygon(surface, hi->tab, hi->tab_size, top_c, clipper);
    ei_draw_polygon(surface, lo->tab, lo->tab_size, bot_c, clipper);
    free(hi->tab); free(hi); free(lo->tab); free(lo);

    ei_rect_t r_int = {{rect.top_left.x , rect.top_left.y },
                       {rect.size.width , rect.size.height }};
    ei_polygone_t* ct = rect_frame(r_int, whole, bw);
    ei_draw_polygon(surface, ct->tab, ct->tab_size, color, clipper);
    free(ct->tab); free(ct);

    if (text) {
        int tw, th;
        hw_text_compute_size(text, font, &tw, &th);
        ei_point_t tp = { r_int.top_left.x + (r_int.size.width - tw) / 2,
                          r_int.top_left.y + (r_int.size.height - th) / 2 };
        if (relief == ei_relief_sunken)
        {
            tp.x += 2;
            tp.y += 2;
        }

        ei_rect_t text_clip = r_int;
        if (clipper != NULL) {
            text_clip = ei_rect_intersection(&text_clip, clipper);
        }

        if (text_clip.size.width > 0 && text_clip.size.height > 0) {
            ei_draw_text(surface, &tp, text, font, text_color, &text_clip);
        }
    }
}

void frame_drawfunc(ei_widget_t widget, ei_surface_t surface, ei_surface_t pick_surface, ei_rect_t* clipper) {
    ei_frame_t* frame=(ei_frame_t*)widget;

    //Pour mettre du relief
    if (frame->relief != ei_relief_none && frame->border_width > 0) {
        draw_frame(surface,frame->bg_color,widget->screen_location,frame->border_width,0,frame->relief,frame->text,frame->text_font,frame->text_color, clipper);
    } else {
        ei_fill(surface, &frame->bg_color, &widget->screen_location);
        if (frame->text != NULL) {
            int tw, th;
            hw_text_compute_size(frame->text, frame->text_font, &tw, &th);
            ei_point_t tp = {
                widget->screen_location.top_left.x + (widget->screen_location.size.width  - tw) / 2,
                widget->screen_location.top_left.y + (widget->screen_location.size.height - th) / 2
            };
            ei_draw_text(surface, &tp, frame->text, frame->text_font, frame->text_color, clipper);
        }
    }
    // ajout d'une img si possible
    if (frame->img != NULL) {
        ei_rect_t src = frame->img_rect ? *frame->img_rect : hw_surface_get_rect(frame->img);
        ei_rect_t content = widget->content_rect ? *widget->content_rect : widget->screen_location;
        ei_rect_t dst = ei_place_anchor_in_rect(content, src.size, frame->img_anchor);
        // on clip la destination avec la source(content)
        ei_rect_t clipped = ei_rect_intersection(&dst, &content);

        if (clipper != NULL)
            // on le restreint si besoin
            clipped = ei_rect_intersection(&clipped, clipper);

        if (clipped.size.width > 0 && clipped.size.height > 0) {
            // on clip la source
            ei_rect_t clipped_src = src;
            clipped_src.top_left.x += clipped.top_left.x - dst.top_left.x;
            clipped_src.top_left.y += clipped.top_left.y - dst.top_left.y;
            clipped_src.size = clipped.size;
            hw_surface_lock(frame->img);
            ei_copy_surface(surface, &clipped, frame->img, &clipped_src, false);
            hw_surface_unlock(frame->img);
        }
    }

    ei_impl_widget_draw_children(widget,surface,pick_surface,clipper); // On appelle le draw pour les enfants de la frame
}

void frame_geomnotifyfunc(ei_widget_t widget) {
    ei_frame_t* frame = (ei_frame_t*)widget;
    int bw = frame->border_width;

    frame->inner_rect.top_left.x = widget->screen_location.top_left.x + bw;
    frame->inner_rect.top_left.y = widget->screen_location.top_left.y + bw;
    frame->inner_rect.size.width = widget->screen_location.size.width  - 2 * bw;
    frame->inner_rect.size.height = widget->screen_location.size.height - 2 * bw;

    widget->content_rect = &frame->inner_rect;
}

ei_widgetclass_t* frame_widgetclass(void)
{
    static ei_widgetclass_t klass = {
        "frame",
        frame_allocfunc,
        frame_releasefunc,
        frame_drawfunc,
        frame_setdefaultsfunc,
        frame_geomnotifyfunc,
        NULL
    };
    return &klass;
}
