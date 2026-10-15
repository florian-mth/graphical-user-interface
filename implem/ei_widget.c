#include "ei_widget.h"
#include "ei_implementation.h"
#include "ei_application.h"
#include "hw_interface.h"
#include "ei_types.h"
#include "ei_types.h"
#include "ei_event.h"
#include "ei_event_prime.h"
#include "ei_utils.h"
#include "ei_widget_attributes.h"
#include "hw_interface.h"
#include "widget_button_class.h"
#include "widget_toplevel_class.h"
#include "ei_application.h"
#include "widget_entry_class.h"


ei_widget_t ei_widget_create (ei_const_string_t	class_name, ei_widget_t parent,
        ei_user_param_t user_data, ei_widget_destructor_t destructor)
{
    if (parent == NULL)// si parent est null, on ne peut pas créer le widget car seule la racine n'a pas de parent
        return NULL;

    ei_widgetclass_t *klass = ei_widgetclass_from_name(class_name); // on récupère la classe liée au nom de ce qu'on veut implementer
    if (klass == NULL || klass->allocfunc == NULL) // si la classe n'existe pas ou qu'elle n'a pas d'allocfunc, on return
        return NULL;

    ei_widget_t widget = klass->allocfunc(); // on crée une instance de la classe
    if (widget == NULL)
        return NULL;
    // on init tous les parametres
    widget->wclass = klass;
    widget->user_data = user_data;
    widget->destructor = destructor;
    widget->parent = parent;
    widget->children_head = NULL;
    widget->children_tail = NULL;
    widget->next_sibling = NULL;
    widget->geom_params = NULL;
    widget->screen_location = ei_rect_zero();
    widget->content_rect = &widget->screen_location;

    if (parent->children_tail == NULL) { //si le parent n'a pas d'enfant on le met simplement dans la liste chainée
        parent->children_head = widget;
        parent->children_tail = widget;
    } else {
        parent->children_tail->next_sibling = widget; // sinon on l'ajoute à la liste et la queue devient celui qu'on a ajouté
        parent->children_tail = widget;
    }

    if (klass->setdefaultsfunc != NULL) //on init le reste des var
        klass->setdefaultsfunc(widget);

    // Bindings selon la classe du widget
    if (strcmp(class_name, "button") == 0) {
        ei_bind(ei_ev_mouse_buttondown, widget, NULL,  button_on_press,   NULL);
        ei_bind(ei_ev_mouse_buttonup, NULL, "all", button_on_release, NULL);
        ei_bind(ei_ev_mouse_move,       NULL,  "all", button_on_move,    NULL);
    }

    if (strcmp(class_name, "toplevel") == 0) {
        ei_toplevel_t* t = (ei_toplevel_t*)widget;
        ei_bind(ei_ev_mouse_buttondown, widget, NULL,  toplevel_on_move,      NULL);
        ei_bind(ei_ev_mouse_buttonup,   NULL,  "all", toplevel_on_release,   NULL);
        ei_bind(ei_ev_mouse_buttondown, t->size_button,  NULL,  toplevel_resize,         NULL);
        ei_bind(ei_ev_mouse_move,       NULL,  "all", toplevel_on_drag,      NULL);
        ei_bind(ei_ev_mouse_move,       NULL,  "all", toplevel_resize_drag,  NULL);
        ei_bind(ei_ev_mouse_buttonup,   NULL,  "all", toplevel_resize_release, NULL);
    }

    if (strcmp(class_name, "entry") == 0) {
        ei_bind(ei_ev_mouse_buttondown, widget, NULL,  entry_on_click,      NULL);
        ei_bind(ei_ev_text_input,       NULL,  "all", entry_on_text_input, NULL);
        ei_bind(ei_ev_keydown,          NULL,  "all", entry_on_keydown,    NULL);
        ei_bind(    ei_ev_none,    NULL,    "all",    entry_blink_handler,    NULL);
    }
    widget->pick_id = next_pick_id++;
    picking_table[widget->pick_id] = widget;
    widget->pick_color = (ei_color_t) {
    .red = (widget-> pick_id) & 0xff, // on met le rouge dans les bits de poids forts
    .green =(widget->pick_id>>8) & 0xff , // le vert dans des bits plus faibles
    .blue = (widget->pick_id >>16) & 0xff , // le bleu dans les bits de poids minimum
    .alpha =0xff }; // couleurs vives donc transparence au minimum

    return widget;
}
//Ancienne façon de pick un widget de manière géométrique et pas avec offscreen picking
// ei_widget_t		ei_widget_pick			(ei_point_t*		where)
// {
//     ei_widget_t root = ei_app_root_widget();
//     ei_widget_t stack[1024];
//     ei_widget_t picked = NULL;
//     int top = 0;
//
//     if (where == NULL || root == NULL)
//         return NULL;
//
//     if (!(where->x >= root->screen_location.top_left.x &&
//           where->y >= root->screen_location.top_left.y &&
//           where->x < root->screen_location.top_left.x + root->screen_location.size.width &&
//           where->y < root->screen_location.top_left.y + root->screen_location.size.height))
//         return NULL;
//
//     stack[top++] = root;
//
//     while (top > 0) {
//         ei_widget_t current = stack[--top];
//         ei_widget_t child = current->children_tail;
//
//         while (child != NULL) {
//             if (child->geom_params != NULL &&
//                 where->x >= child->screen_location.top_left.x &&
//                 where->y >= child->screen_location.top_left.y &&
//                 where->x < child->screen_location.top_left.x + child->screen_location.size.width &&
//                 where->y < child->screen_location.top_left.y + child->screen_location.size.height) {
//                 picked = child;
//                 stack[top++] = child;
//                 break;
//             }
//
//             ei_widget_t prev = NULL;
//             ei_widget_t cur = current->children_head;
//             while (cur != NULL && cur->next_sibling != child) {
//                 cur = cur->next_sibling;
//             }
//             child =cur;
//         }
//     }
//
//     return picked;
//     // si le pointeur est bien dans la fenêtre mais n'est sur aucun widget =/= de root, la fonction renvoie NULL et pas root.
// }

ei_widget_t ei_widget_pick (ei_point_t*	where)
{
    ei_widget_t root = ei_app_root_widget();
    ei_widget_t picked = NULL;

    if (where == NULL || root == NULL)
        return NULL;

    if (!(where->x >= root->screen_location.top_left.x &&
          where->y >= root->screen_location.top_left.y &&
          where->x < root->screen_location.top_left.x + root->screen_location.size.width &&
          where->y < root->screen_location.top_left.y + root->screen_location.size.height))
        return NULL;

    ei_surface_t pick_surface = ei_app_pick_surface();
    if (pick_surface == NULL){ return NULL;}

    hw_surface_lock(pick_surface);
    uint8_t* buffer = hw_surface_get_buffer(pick_surface);
    ei_size_t size_ps = hw_surface_get_size(pick_surface);
    int red, green, blue, alpha;
    hw_surface_get_channel_indices(pick_surface, &red, &green, &blue, &alpha);

    uint8_t* ptr_x_y = buffer + 4*(where->y *size_ps.width + where->x ); // multiplication par 4 car on manipule des octets car 1 pixel = 4octets RGBA
    uint32_t id = ((uint32_t)ptr_x_y[red]) |
                  ((uint32_t)ptr_x_y[green] << 8) |
                  ((uint32_t)ptr_x_y[blue] << 16);
    //on ignore la transparence car ce sont des couleurs vives

    hw_surface_unlock(pick_surface);
    picked = picking_table[id];
    return picked;
    // si le pointeur est bien dans la fenêtre mais n'est sur aucun widget =/= de root, la fonction renvoie NULL et pas root.
}

void ei_widget_destroy (ei_widget_t widget) {
    if (widget == NULL) return;
    if (widget->parent == NULL) return; // on detruit pas la racine

    if (widget->geom_params != NULL) {
        ei_app_invalidate_rect(&widget->screen_location);
    }
    //détruit enfant
    if (widget->children_head != NULL) {
        ei_widget_destroy(widget->children_head);
    }
    //retire le widget de la liste des enfants et du parent
    ei_widget_t parent = widget->parent;
    if (parent->children_head == widget) {
        parent->children_head = widget->next_sibling;
    } else {
        ei_widget_t prev = parent->children_head;
        while (prev!= NULL && prev->next_sibling != widget) {
            prev = prev->next_sibling;
        }
        if (prev != NULL) {
            prev->next_sibling = widget->next_sibling;
        }
    }

    if (parent->children_tail == widget) {
        if (parent->children_head == NULL) {
            parent->children_tail = NULL;
        } else {
            ei_widget_t child = parent->children_head;
            while (child->next_sibling != NULL) {
                child = child->next_sibling;
            }
            parent->children_tail = child;
        }
    }

    widget->parent = NULL;
    widget->next_sibling = NULL;

    ei_geometrymanager_unmap(widget);  //libère geometrymanager
    // on unbind les bindings faites sur le widget
    ei_widget_unbind_all(widget);

    if (widget->destructor != NULL) {
        widget->destructor(widget);
    }
    if (widget->wclass != NULL && widget->wclass->releasefunc != NULL) {
        widget->wclass->releasefunc(widget);
    }
    picking_table[widget->pick_id] = NULL;
    free(widget);
}

bool ei_widget_is_displayed(ei_widget_t widget)
{
    if (widget == NULL) return false;
    if (widget->geom_params == NULL) return false;
    return true;
}