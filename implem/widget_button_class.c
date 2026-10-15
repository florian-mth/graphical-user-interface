#include "ei_types.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "widget_button_class.h"
#include "ei_draw.h"
#include "ei_widget_configure.h"
#include "ei_application.h"
#include "ei_event.h"
#include "intersection.h"
#include "ei_geometrymanager_prime.h"
static ei_button_t* g_active_button = NULL;  // Pointeur vers le bouton activé

// Renvoie un tableau de points dessinant un arc dans le sens trigo
ei_point_t* arc(ei_point_t center, int radius, float angle_start, float angle_end, int* num_pts) {
    int n = abs((int)(angle_end - angle_start)) / 5 + 1;
    if (n < 2) n = 2;
    ei_point_t* pts = malloc(n * sizeof(ei_point_t));
    *num_pts = n;

    float step = (angle_end - angle_start) / (float)(n - 1);
    for (int i = 0; i < n; i++) {
        double rad = (angle_start + i * step) * M_PI / 180.0;
        pts[i].x = center.x + (int)lround(radius * cos(rad));
        pts[i].y = center.y - (int)lround(radius * sin(rad));
    }
    return pts;
}

ei_polygone_t* rounded_frame(ei_rect_t rect, int radius, button_part_t part, int bw) {
    int max_r = (rect.size.width < rect.size.height) ? rect.size.width / 2 : rect.size.height / 2;
    if (radius > max_r) radius = max_r;

    // Centres de chaque coin arrondi
    ei_point_t c_tl = {rect.top_left.x + radius, rect.top_left.y + radius};
    ei_point_t c_tr = {rect.top_left.x + rect.size.width - radius, rect.top_left.y + radius};
    ei_point_t c_br = {rect.top_left.x + rect.size.width - radius, rect.top_left.y + rect.size.height - radius};
    ei_point_t c_bl = {rect.top_left.x + radius, rect.top_left.y + rect.size.height - radius};

    // On définit le nombre de points retournés à 200
    ei_point_t* points = malloc(200 * sizeof(ei_point_t));
    int pos = 0, n = 0;
    ei_point_t* tmp;

    if (part == whole) {
        tmp = arc(c_tl, radius, 90, 180, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
        tmp = arc(c_bl, radius, 180, 270, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
        tmp = arc(c_br, radius, 270, 360, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
        tmp = arc(c_tr, radius, 0, 90, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
    } else {
        int h = bw;
        if (part == top_only) {
            tmp = arc(c_bl, radius, 225, 180, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
            tmp = arc(c_tl, radius, 180, 90, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
            tmp = arc(c_tr, radius, 90, 45, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);

            tmp = arc(c_tr, radius-h, 45, 90, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
            tmp = arc(c_tl, radius-h, 90, 180, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
            tmp = arc(c_bl, radius-h, 180, 225, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);

        } else {  // part == bottom-only
            tmp = arc(c_tr, radius, 45, 0, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
            tmp = arc(c_br, radius, 360, 270, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
            tmp = arc(c_bl, radius, 270, 225, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);

            tmp = arc(c_bl, radius-h, 225, 270, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
            tmp = arc(c_br, radius-h, 270, 360, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
            tmp = arc(c_tr, radius-h, 0, 45, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
        }
    }
    ei_polygone_t* res = malloc(sizeof(ei_polygone_t));
    res->tab = points;
    res->tab_size = pos;
    return res;
}

void draw_button(ei_surface_t surface, ei_color_t color, ei_rect_t rect,
                 int bw, int r, ei_relief_t relief, ei_string_t text, ei_font_t font,
                 ei_color_t text_color,ei_anchor_t text_anchor, ei_rect_t* clipper) {

    ei_color_t dark = {color.red/2, color.green/2, color.blue/2, color.alpha}; // Moyenne entre la couleur et le noir
    ei_color_t light = {(color.red+255)/2, (color.green+255)/2, (color.blue+255)/2, color.alpha};  // Moyenne entre la couleur et le blanc
    ei_color_t top_c = (relief == ei_relief_sunken) ? dark : light;
    ei_color_t bot_c = (relief == ei_relief_sunken) ? light : dark;

    ei_polygone_t* hi = rounded_frame(rect, r, top_only, bw);
    ei_polygone_t* lo = rounded_frame(rect, r, bottom_only, bw);
    ei_draw_polygon(surface, hi->tab, hi->tab_size, top_c, clipper);
    ei_draw_polygon(surface, lo->tab, lo->tab_size, bot_c, clipper);
    free(hi->tab);
    free(hi);
    free(lo->tab);
    free(lo);

    ei_rect_t r_int = {{rect.top_left.x + bw, rect.top_left.y + bw},
                       {rect.size.width - 2*bw, rect.size.height - 2*bw}};
    int r_int_val = (r > bw) ? r - bw : 0;
    ei_polygone_t* ct = rounded_frame(r_int, r_int_val, whole, bw);
    ei_draw_polygon(surface, ct->tab, ct->tab_size, color, clipper);
    free(ct->tab);
    free(ct);

    if (text) {
        int tw, th;
        hw_text_compute_size(text, font, &tw, &th);
        ei_size_t text_size = {tw,th};
        ei_rect_t text_rect = ei_place_anchor_in_rect(r_int, text_size, text_anchor);
        ei_point_t tp = text_rect.top_left;
        if (relief == ei_relief_sunken) { tp.x += 2; tp.y += 2; }

        ei_rect_t text_clip = r_int;
        if (clipper != NULL) {
            text_clip = ei_rect_intersection(&text_clip, clipper);
        }
        if (text_clip.size.width > 0 && text_clip.size.height > 0) {
            ei_draw_text(surface, &tp, text, font, text_color, &text_clip);
        }
    }
}

ei_widget_t button_allocfunc(void)
{
    ei_button_t* button = calloc(1, sizeof(ei_button_t));
    return (ei_widget_t)button;
}

void button_releasefunc(ei_widget_t widget)
{
    ei_button_t* button = (ei_button_t*)widget;
    // libère proprement la copie de l'image
    if (button->img != NULL) {
        hw_surface_free(button->img);
        button->img = NULL;
    }
    if (button->img_rect != NULL) {
        free(button->img_rect);
        button->img_rect = NULL;
    }
    if (button->text != NULL) {
        free(button->text);
        button->text = NULL;
    }
    if (g_active_button == button)
        g_active_button = NULL;
}

void button_setdefaultsfunc(ei_widget_t widget)
{
    ei_button_t* button = (ei_button_t*)widget;
    button->bg_color = ei_default_background_color;
    button->border_width = k_default_button_border_width;
    button->corner_radius = k_default_button_corner_radius;
    button->relief = ei_relief_raised;
    button->text = NULL;
    button->text_font = ei_default_font;
    button->text_color = ei_font_default_color;
    button->text_anchor = ei_anc_center;
    button->img = NULL;
    button->img_rect = NULL;
    button->img_anchor = ei_anc_center;
    button->callback = NULL;
    button->user_param = NULL;
    button->pressed = false;

    widget->requested_size = (ei_size_t){0, 0};
    widget->content_rect = &widget->screen_location;
}

void button_drawfunc(ei_widget_t widget, ei_surface_t surface, ei_surface_t pick_surface, ei_rect_t* clipper)
{
    (void)pick_surface;
    ei_button_t* button = (ei_button_t*)widget;
    draw_button(surface, button->bg_color, widget->screen_location, button->border_width, button->corner_radius,
        button->relief, button->text, button->text_font, button->text_color,button->text_anchor, clipper);

    if (pick_surface !=NULL)
    {
        ei_polygone_t* pick_shape =rounded_frame(widget->screen_location, button->corner_radius, whole, button->border_width);
        ei_color_t pick_color = widget->pick_color;
        pick_color.alpha=0xff;
        ei_draw_polygon(pick_surface, pick_shape->tab, pick_shape->tab_size, pick_color, clipper);
        free(pick_shape->tab);
        free(pick_shape);
    }
    if (button->img != NULL) {
        ei_rect_t src = button->img_rect ? *button->img_rect : hw_surface_get_rect(button->img);
        ei_rect_t dst = {
            {widget->screen_location.top_left.x + button->border_width,widget->screen_location.top_left.y + button->border_width},
            src.size
        };
        ei_rect_t vrai_destination = clipper ? ei_rect_intersection(&dst,clipper) : dst;

        if (vrai_destination.size.width > 0 && vrai_destination.size.height > 0) {
            ei_rect_t vrai_src = src;
            vrai_src.top_left.x = vrai_destination.top_left.x - dst.top_left.x;
            vrai_src.top_left.y = vrai_destination.top_left.y - dst.top_left.y;
            vrai_src.size = vrai_destination.size;
            hw_surface_lock(button->img);
            ei_copy_surface(surface, &vrai_destination, button->img, &vrai_src, false);
            hw_surface_unlock(button->img);
        }
    }
    ei_impl_widget_draw_children(widget, surface, pick_surface, clipper);
}

void button_geomnotifyfunc(ei_widget_t widget)
{
    widget->content_rect = &widget->screen_location;
}

ei_widgetclass_t* button_widgetclass(void)
{
    static ei_widgetclass_t klass = {
        "button",
        button_allocfunc,
        button_releasefunc,
        button_drawfunc,
        button_setdefaultsfunc,
        button_geomnotifyfunc,
        NULL
    };
    return &klass;
}

bool button_on_press(ei_widget_t widget, ei_event_t* event, void* user_param){
    if (widget == NULL)
    {
        return false;
    }
    ei_button_t* button = (ei_button_t*)widget;
    button->pressed = true;
    button->relief  = ei_relief_sunken;
    g_active_button = button;  // Un seul bouton peut être activé à la fois. Là on choisit button (le widget avec lequel on interragit)
    // On redessine la zone du bouton
    ei_app_invalidate_rect(&widget->screen_location);
    return true;
}

bool button_on_release(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    if (event == NULL || g_active_button == NULL) {
        return false;
    }

    ei_button_t* button = g_active_button;
    ei_widget_t active_widget = (ei_widget_t)button;

    ei_point_t mouse_where = event->param.mouse.where;
    ei_widget_t picked = ei_widget_pick(&mouse_where);
    bool inside = (picked == active_widget);

    button->pressed = false;
    button->relief = ei_relief_raised;

    if (inside && button->callback != NULL) {
        button->callback(active_widget, event, button->user_param);
    }

    g_active_button = NULL;
    ei_app_invalidate_rect(&active_widget->screen_location);
    return false;
}

bool button_on_move(ei_widget_t widget, ei_event_t* event, void* user_param){
    // Si aucun bouton n'est pressé, on ne fait rien
    if (g_active_button == NULL)
    {
        return false;
    }

    ei_button_t* button = g_active_button;
    ei_widget_t  widg = (ei_widget_t)button;

    ei_point_t mouse_where = event->param.mouse.where;
    ei_widget_t picked = ei_widget_pick(&mouse_where);

    //=== Ancienne façon de définir le booléen inside ===
    // ei_rect_t  button_loc   = widg->screen_location;
    // bool inside = (mouse_where.x >= button_loc.top_left.x &&
    //                mouse_where.y >= button_loc.top_left.y &&
    //                mouse_where.x <  button_loc.top_left.x + button_loc.size.width &&
    //                mouse_where.y <  button_loc.top_left.y + button_loc.size.height);

    bool inside = (picked == widg);

    // Si le pointeur est en dehors du bouton, on change l'apparence du bouton
    if (!inside && button->pressed) {
        button->pressed = false;
        button->relief  = ei_relief_raised;
        ei_app_invalidate_rect(&widg->screen_location);
    }
    if (inside && !button->pressed) {
        button->pressed = true;
        button->relief  = ei_relief_sunken;
        ei_app_invalidate_rect(&widg->screen_location);
    }
    return false;
}
