#include "ei_types.h"
#include "ei_draw.h"
#include "ei_implementation.h"
#include "ei_widget.h"
#include "ei_event.h"
#include "widget_button_class.h"
#include "ei_widget_configure.h"
#include "widget_toplevel_class.h"
#include "ei_placer.h"
#include "ei_application.h"
#include "intersection.h"
static ei_widget_t g_dragged_toplevel = NULL;  // toplevel en cours de déplacement
static ei_point_t  g_drag_offset;  // offset entre le coin du toplevel et la souris (pour redessiner)
static ei_widget_t g_resized_toplevel = NULL;
static ei_point_t  g_resize_start_mouse;   // position souris au début du resize
static ei_size_t   g_resize_start_size;    // taille du toplevel au début du resize


bool toplevel_close(ei_widget_t widget, ei_event_t* event, void* user_param) {
    (void)event;
    (void)user_param;

    if (widget == NULL || widget->parent == NULL) return false;
    ei_widget_t parent = widget->parent;

    if (parent->wclass == NULL || parent->wclass->releasefunc==NULL) return false;
    ei_widget_destroy(parent);
    return true;
}

// Début du déplacement : on clic sur le header
bool toplevel_on_move(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    if (widget == NULL) return false;

    ei_toplevel_t* t = (ei_toplevel_t*)widget;
    ei_point_t where = event->param.mouse.where;
    ei_rect_t  loc   = widget->screen_location;

    // Vérifie que le clic est dans le header
    bool in_header = (where.x >= loc.top_left.x &&
                      where.x <= loc.top_left.x + loc.size.width &&
                      where.y >= loc.top_left.y &&
                      where.y <= loc.top_left.y + t->header_height);

    if (!in_header) return false;

    // Stocke le toplevel actif et l'offset
    g_dragged_toplevel = widget;
    g_drag_offset.x = where.x - loc.top_left.x;
    g_drag_offset.y = where.y - loc.top_left.y;

    // Chevauche ses frères s'il n'est pas la queue de son père
    ei_widget_t parent = widget->parent;

    if (parent != NULL && parent->children_tail != widget) {


        if (parent->children_head == widget) {
            parent->children_head = widget->next_sibling;
        } else {
            ei_widget_t prec = parent->children_head;

            while (prec != NULL && prec->next_sibling != widget) {
                prec = prec->next_sibling;
            }

            // Pour ne pas écraser les autres frères
            if (prec != NULL) {
                prec->next_sibling = widget->next_sibling;
            }
        }


        parent->children_tail->next_sibling = widget;
        parent->children_tail = widget;
        widget->next_sibling = NULL;

        ei_app_invalidate_rect(&parent->screen_location);
    }
    return true;
}

// Fin du déplacement : on relache le bouton souris
bool toplevel_on_release(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    g_dragged_toplevel = NULL;
    return false;
}

// Déplacement : on bouge la souris
bool toplevel_on_drag(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    if (g_dragged_toplevel == NULL) return false;

    ei_point_t where = event->param.mouse.where;

    // Calcule la nouvelle position
    int new_x = where.x - g_drag_offset.x;
    int new_y = where.y - g_drag_offset.y;

    if (new_y < 0)
    {
        new_y = 0;
    }
    if (new_x < 0)
    {
        new_x = 0;
    }

    ei_rect_t root_surface = hw_surface_get_rect(ei_app_root_surface());

    if (new_x > root_surface.top_left.x + root_surface.size.width - g_dragged_toplevel->screen_location.size.width)
    {
        new_x = root_surface.top_left.x + root_surface.size.width - g_dragged_toplevel->screen_location.size.width;
    }
    if (new_y > root_surface.top_left.y + root_surface.size.height - g_dragged_toplevel->screen_location.size.height)
    {
        new_y = root_surface.top_left.y + root_surface.size.height - g_dragged_toplevel->screen_location.size.height;
    }

    // Invalide l'ancienne position
    ei_app_invalidate_rect(&g_dragged_toplevel->screen_location);

    // Met à jour la position via le placer
    ei_place_xy(g_dragged_toplevel, new_x, new_y);

    // Invalide la nouvelle position
    ei_app_invalidate_rect(&g_dragged_toplevel->screen_location);

    return true;
}

bool toplevel_resize(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    if (widget == NULL) return false;

    ei_toplevel_t* t = (ei_toplevel_t*)widget;
    // widget ici est le bouton resize, pas le toplevel
    // on récupère le toplevel = parent du bouton
    ei_widget_t toplevel_widget = widget->parent;

    // Stocke l'état initial
    g_resized_toplevel = toplevel_widget;
    g_resize_start_mouse = event->param.mouse.where;
    g_resize_start_size = toplevel_widget->screen_location.size;
    return false;  // Si on retourne true, la chaine s'arrête après l'exécution de cette fonction et n'exécute pas les suivantes
}

bool toplevel_resize_drag(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    if (g_resized_toplevel == NULL) return false;

    ei_toplevel_t* t    = (ei_toplevel_t*)g_resized_toplevel;
    ei_point_t     where = event->param.mouse.where;

    // Calcule le delta depuis le début du resize
    int delta_x = where.x - g_resize_start_mouse.x;
    int delta_y = where.y - g_resize_start_mouse.y;

    // Nouvelle taille
    // Si on ne peut rien modifier :
    if (t->resizable == 0)
    {
        return true; // On ne change rien
    }

    // Si on peut modifier uniquement les abscisses :
    if (t->resizable == ei_axis_x)
    {
        delta_y = 0;
    }

    // Si on peut modifier uniquement les ordonnées :
    if (t->resizable == ei_axis_y)
    {
        delta_x = 0;
    }

    int new_w = g_resize_start_size.width  + delta_x;
    int new_h = g_resize_start_size.height + delta_y;

    // Ne redimenssionne pas plus grand que root
    ei_rect_t root_surface = hw_surface_get_rect(ei_app_root_surface());
    if (new_w > root_surface.size.width - t->widget.geom_params->x)
    {
        new_w = root_surface.size.width - t->widget.geom_params->x;
    }
    if (new_h > root_surface.size.height - t->widget.geom_params->y)
    {
        new_h  = root_surface.size.height - t->widget.geom_params->y;
    }

    // Respecte la taille minimale
    if (new_w < t->min_size.width)  new_w = t->min_size.width;
    if (new_h < t->min_size.height) new_h = t->min_size.height;

    // Invalide l'ancienne position
    ei_app_invalidate_rect(&g_resized_toplevel->screen_location);

    // Met à jour via le placer
    ei_place(g_resized_toplevel, NULL,
             NULL, NULL,
             &new_w, &new_h,
             NULL, NULL, NULL, NULL);

    // Invalide la nouvelle position
    ei_app_invalidate_rect(&g_resized_toplevel->screen_location);

    return true;
}

bool toplevel_resize_release(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    g_resized_toplevel = NULL;
    return false;
}

bool toplevel_is_decoration_widget(ei_widget_t toplevel_widget, ei_widget_t child)
{
    if (toplevel_widget == NULL || child == NULL || toplevel_widget->wclass != toplevel_widgetclass()) {
        return false;
    }

    ei_toplevel_t* toplevel = (ei_toplevel_t*)toplevel_widget;
    return child == toplevel->close_button || child == toplevel->size_button;
}

// Dessine un rectangle avec les angles supérieurs arrondis
ei_polygone_t* rounded_straight_frame(ei_rect_t rect, int radius) {

    ei_point_t c_tl = {rect.top_left.x + radius, rect.top_left.y + radius};
    ei_point_t c_tr = {rect.top_left.x + rect.size.width - radius, rect.top_left.y + radius};

    ei_point_t* points = malloc(200 * sizeof(ei_point_t));
    int pos = 0, n = 0;
    ei_point_t* tmp;

    tmp = arc(c_tl, radius, 180, 90, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
    tmp = arc(c_tr, radius, 90, 0, &n); for(int i=0; i<n; i++) points[pos++] = tmp[i]; free(tmp);
    points[pos++] = (ei_point_t){rect.top_left.x + rect.size.width, rect.top_left.y + rect.size.height};
    points[pos++] = (ei_point_t){rect.top_left.x, rect.top_left.y + rect.size.height};

    ei_polygone_t* res = malloc(sizeof(ei_polygone_t));
    res->tab = points;
    res->tab_size = pos;
    return res;
}

void draw_toplevel(ei_surface_t surface, int r, ei_color_t headercolor,
                   ei_color_t bg_color, ei_rect_t rect,
                   int header_height, int border_width,
                   ei_string_t title, ei_font_t font,
                   ei_color_t title_color, ei_rect_t* clipper)
{
    // headercolor.alpha = bg_color.alpha; Permet d'unifier la toplevel mais est trop contraignante
    // On veut pouvoir dissocier la transparence du header et du background

    ei_rect_t rect_header = {
        .top_left = rect.top_left,
        .size = { rect.size.width, header_height }
    };
    ei_rect_t left_border_rect= {
        .top_left = {rect.top_left.x, rect.top_left.y+header_height},
        .size = {border_width,rect.size.height - header_height}
    };

    ei_rect_t right_border_rect= {
        .top_left = {rect.top_left.x +rect.size.width - border_width, rect.top_left.y+header_height},
        .size = {border_width,rect.size.height - header_height}
    };
    ei_rect_t bottom_border_rect= {
        .top_left = {rect.top_left.x+border_width,rect.top_left.y + rect.size.height - border_width},
        .size = {rect.size.width -2*border_width,border_width}
    };

    ei_polygone_t* header = rounded_straight_frame(rect_header,r);
    ei_draw_polygon(surface, header->tab, header->tab_size, headercolor, clipper);
    free(header->tab); free(header);

    // Résout le problème de toplevel fille de toplevel
    ei_point_t left_border[4] = {
        {left_border_rect.top_left.x, left_border_rect.top_left.y},
        {left_border_rect.top_left.x +left_border_rect.size.width,left_border_rect.top_left.y},
        {left_border_rect.top_left.x +left_border_rect.size.width,left_border_rect.top_left.y+left_border_rect.size.height},
        {left_border_rect.top_left.x,left_border_rect.top_left.y +left_border_rect.size.height}
    };
    ei_draw_polygon(surface, left_border, 4, headercolor, clipper);

    ei_point_t right_border[4] = {
        {right_border_rect.top_left.x, right_border_rect.top_left.y},
        {right_border_rect.top_left.x +right_border_rect.size.width, right_border_rect.top_left.y},
        {right_border_rect.top_left.x+right_border_rect.size.width, right_border_rect.top_left.y +right_border_rect.size.height},
        {right_border_rect.top_left.x, right_border_rect.top_left.y +right_border_rect.size.height}
    };
    ei_draw_polygon(surface, right_border, 4, headercolor, clipper);

    ei_point_t bottom_border[4] = {
        {bottom_border_rect.top_left.x, bottom_border_rect.top_left.y},
        {bottom_border_rect.top_left.x +bottom_border_rect.size.width, bottom_border_rect.top_left.y},
        {bottom_border_rect.top_left.x +bottom_border_rect.size.width,bottom_border_rect.top_left.y +bottom_border_rect.size.height},
        {bottom_border_rect.top_left.x,bottom_border_rect.top_left.y+ bottom_border_rect.size.height}
    };
    ei_draw_polygon(surface, bottom_border, 4, headercolor, clipper);

    ei_rect_t body = {
        .top_left = {rect.top_left.x+ border_width, rect.top_left.y + header_height},
        .size = {rect.size.width - 2*border_width, rect.size.height - border_width - header_height}
    };

    ei_point_t* points_body = malloc(4* sizeof(ei_point_t));
    points_body[0] = (ei_point_t){body.top_left.x, body.top_left.y};
    points_body[1] = (ei_point_t){body.top_left.x + body.size.width, body.top_left.y};
    points_body[2] = (ei_point_t){body.top_left.x + body.size.width, body.top_left.y + body.size.height};
    points_body[3] = (ei_point_t){body.top_left.x, body.top_left.y + body.size.height};

    ei_polygone_t body_poly = {points_body, 4};
    ei_draw_polygon(surface, body_poly.tab, body_poly.tab_size, bg_color, clipper);
    free(points_body);

    if (title) {
        int tw, th;
        hw_text_compute_size(title, font, &tw, &th);
        ei_point_t tp = {
            rect.top_left.x + r + 30,
            rect.top_left.y + (header_height - th) / 2
        };
        ei_draw_text(surface, &tp, title, font, title_color, clipper);
    }

}

ei_widget_t toplevel_allocfunc(void) {
    ei_toplevel_t* toplevel = calloc(1, sizeof(ei_toplevel_t));
    return (ei_widget_t)toplevel;
}

void toplevel_releasefunc(ei_widget_t widget) {
    ei_toplevel_t* toplevel = (ei_toplevel_t*)widget;
    if (toplevel->title != NULL)
        free(toplevel->title);
}

void toplevel_setdefaultsfunc(ei_widget_t widget) {
    ei_toplevel_t* t = (ei_toplevel_t*)widget;
    t->bg_color             = (ei_color_t){ 0xA0, 0xA0, 0xA0, 0xff};
    t->header_color         = (ei_color_t){0x2F, 0x2F, 0x2F, 0xff};
    t->header_corner_radius = k_default_button_corner_radius;
    t->header_height        = 24;
    t->border_width         = 4;
    t->title                = NULL;
    t->text_font            = ei_default_font;
    t->text_color           = (ei_color_t){0xFF, 0xFF, 0xFF, 0xFF};
    t->text_anchor          = ei_anc_center;
    t->img                  = NULL;
    t->img_rect             = NULL;
    t->img_anchor           = ei_anc_center;
    widget->content_rect    = &widget->screen_location;
    t->closable  = true;
    t->resizable = ei_axis_both;
    t->min_size  = (ei_size_t){160, 120};
    t->requested_size = (ei_size_t){320, 240};
    // ajout d'un requesed_size pour le toplevel
    widget->requested_size = (ei_size_t){
        t->requested_size.width  + 2 * t->border_width,
        t->requested_size.height + t->header_height + t->border_width
    };

    t->close_button = ei_widget_create("button", widget, NULL, NULL);
    ei_button_configure(t->close_button,
                        &(ei_size_t){12, 12},          // petite taille
                        &(ei_color_t){0xFF, 0x00, 0x00, 0xFF},  // rouge
                        &(int){1}, &(int){8},           // border, rayon
                        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                        &(ei_callback_t){toplevel_close}, NULL);
    // Rond rouge en haut à gauche
    ei_place_xy(t->close_button, 6, 6);

    t->size_button = ei_widget_create("button", widget, NULL, NULL);
    ei_button_configure(t->size_button,
                        &(ei_size_t){12, 12},
                        &t->header_color,
                        &(int){1}, &(int){0},
                        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                        NULL, NULL);

    // En bas à droite
    ei_place(t->size_button,
             &(ei_anchor_t){ei_anc_southeast},
             &(int){0}, &(int){0},
             NULL, NULL,
             &(float){1.0f}, &(float){1.0f},
             NULL, NULL);
}

void toplevel_drawfunc(ei_widget_t widget, ei_surface_t surface,
                       ei_surface_t pick_surface, ei_rect_t* clipper)
{
    ei_toplevel_t* toplevel = (ei_toplevel_t*)widget;
    draw_toplevel(surface,
                  toplevel->header_corner_radius,
                  toplevel->header_color,
                  toplevel->bg_color,
                  widget->screen_location,
                  toplevel->header_height,
                  toplevel->border_width,
                  toplevel->title,
                  toplevel->text_font,
                  toplevel->text_color,
                  clipper);

    if (pick_surface!=NULL)
    {
        ei_color_t pick_color = widget->pick_color;
        ei_polygone_t* pick_shape = rounded_frame(widget->screen_location, toplevel->header_corner_radius,whole,toplevel->border_width);
        pick_color.alpha = 0xff;
        ei_draw_polygon(pick_surface, pick_shape->tab, pick_shape->tab_size, pick_color, clipper);
        free(pick_shape->tab);
        free(pick_shape);
    }
    ei_impl_widget_draw_children(widget, surface, pick_surface, clipper); // Attention ordre important ! Il faut dessiner les enfants de la toplevel après l'appel de dessin dans l'offscreen.
}

void toplevel_geomnotifyfunc(ei_widget_t widget)
{
    ei_toplevel_t* toplevel = (ei_toplevel_t*)widget;
    int bw = toplevel->border_width;
    int hh = toplevel->header_height;

    toplevel->content_rect_storage.top_left.x = widget->screen_location.top_left.x + bw;
    toplevel->content_rect_storage.top_left.y = widget->screen_location.top_left.y + hh;

    toplevel->content_rect_storage.size.width =widget->screen_location.size.width - 2 * bw;
    toplevel->content_rect_storage.size.height =widget->screen_location.size.height - hh - bw;

    if (toplevel->content_rect_storage.size.width < 0)
        toplevel->content_rect_storage.size.width = 0;

    if (toplevel->content_rect_storage.size.height < 0)
        toplevel->content_rect_storage.size.height = 0;

    widget->content_rect = &toplevel->content_rect_storage;
}

ei_widgetclass_t* toplevel_widgetclass(void)
{
    static ei_widgetclass_t kclass = {
        "toplevel",
        toplevel_allocfunc,
        toplevel_releasefunc,
        toplevel_drawfunc,
        toplevel_setdefaultsfunc,
        toplevel_geomnotifyfunc,
        NULL
    };
    return &kclass;
}

ei_rect_t rect_union(ei_rect_t a, ei_rect_t b)
{
    int left = (a.top_left.x < b.top_left.x) ? a.top_left.x : b.top_left.x;
    int top = (a.top_left.y < b.top_left.y) ? a.top_left.y : b.top_left.y;
    int right_a = a.top_left.x + a.size.width;
    int right_b = b.top_left.x + b.size.width;
    int bottom_a = a.top_left.y + a.size.height;
    int bottom_b = b.top_left.y + b.size.height;
    int right = (right_a > right_b) ? right_a : right_b;
    int bottom = (bottom_a > bottom_b) ? bottom_a : bottom_b;
    return (ei_rect_t){.top_left = {left, top}, .size = {right - left, bottom - top}};
}
