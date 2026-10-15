#include "ei_application.h"
#include "ei_event.h"
#include "widget_frame_class.h"
#include "widget_button_class.h"
#include "ei_geometrymanager.h"
#include "ei_placer.h"
#include "ei_geometrymanager_prime.h"
#include "ei_event_prime.h"
#include "intersection.h"
#include "widget_toplevel_class.h"
#include "widget_entry_class.h"



static ei_surface_t root_surface = NULL; //init root_surface
static ei_surface_t pick_surface = NULL; //init pick_surface
static ei_widget_t root_widget = NULL; // init le widget root
static ei_linked_rect_t* rects_todo = NULL; // listes chainées des zones à rafraichir
static bool stop_condition = true;

void ei_app_create(ei_size_t main_window_size, bool fullscreen)
{
    hw_init();
    root_surface = hw_create_window(main_window_size, fullscreen); // recupere la surface de la main window
    pick_surface = hw_surface_create(root_surface, hw_surface_get_size(root_surface), false); //recupere la surace de la pick_surface
    ei_default_font = hw_text_font_create(ei_default_font_filename, ei_style_normal, ei_font_default_size);

    ei_widgetclass_register(frame_widgetclass()); // on enregistre la widgetclass frame
    ei_widgetclass_register(button_widgetclass()); // on enregistre la widgetclass button
    ei_widgetclass_register(toplevel_widgetclass()); // on enregistre la widgetclass button
    ei_widgetclass_register(entry_widgetclass()); // on enregistre la classe entry

    root_widget = frame_widgetclass()->allocfunc(); // on alloue la memoire
    root_widget->wclass = frame_widgetclass(); // donne sa classe
    root_widget->parent = NULL;
    root_widget->children_head = NULL;
    root_widget->children_tail = NULL;
    root_widget->next_sibling = NULL;
    root_widget->geom_params = NULL;
    root_widget->screen_location = hw_surface_get_rect(root_surface);
    root_widget->requested_size = root_widget->screen_location.size;
    root_widget->content_rect = &root_widget->screen_location;
    root_widget->wclass->setdefaultsfunc(root_widget);

    ei_geometrymanager_register(placer_widgetclass()); // on enregistre le placer comme pour la widgetclass
}

void ei_app_free(void)
{
    if (root_widget && root_widget->wclass && root_widget->wclass->releasefunc)
        root_widget->wclass->releasefunc(root_widget);
    free(root_widget);
    if (pick_surface) hw_surface_free(pick_surface);
    if (root_surface) hw_surface_free(root_surface);
    hw_quit();
}


void ei_app_run(void)
{
    ei_event_t event = {.type = ei_ev_none};
    ei_event_t event_other_type = {.type = ei_ev_none};
    bool waiting_event = false;
    double last_time = 0.0;
    const double min_frame_period = 1.0 / 120.0;

    hw_surface_lock(root_surface);
    hw_surface_lock(pick_surface);
    root_widget->wclass->drawfunc(root_widget, root_surface, pick_surface, NULL); //on draw la root et tous ses enfants
    hw_surface_unlock(pick_surface);
    hw_surface_unlock(root_surface);
    hw_surface_update_rects(root_surface, NULL);
    last_time = hw_now();

    // Permet de corriger les artefacts involontaires dus au lancement de la boucle principale
    while (rects_todo != NULL) {
        ei_linked_rect_t* next = rects_todo->next;
        free(rects_todo);
        rects_todo = next;
    }
    // On est maintenant sur des bases saines

    while (stop_condition)
    {
        if ( (event.type == ei_ev_close) || ((event.type == ei_ev_keydown) && (event.param.key_code == SDLK_ESCAPE)))
        {
            ei_app_quit_request();
            return ;
        }

        if (waiting_event) {
            event = event_other_type;
            waiting_event = false;
        } else {
            if (hw_event_has_events()) {
                hw_event_wait_next(&event);
            } else {
                event.type = ei_ev_none;
            }
        }

        // Pour éviter le clignotement de la fenêtre: pendant une rafale de mouse_move, on ne garde que la dernière position.

        // on lit des events en avance avec hw_event_wait_next(&next_event),
        // tant que c’est ei_ev_mouse_move, on les écrase (on garde le dernier),
        // dès qu’on rencontre un event d’un autre type: on s'arrête
        // au début du tour de boucle suivant, cet event est traité en priorité, puis has_event_other_type repasse à false.
        if (event.type == ei_ev_mouse_move) {
            ei_event_t next_event = {.type = ei_ev_none};
            while (hw_event_has_events()) {
                hw_event_wait_next(&next_event);
                if (next_event.type == ei_ev_mouse_move) {
                    event = next_event;
                } else {
                    event_other_type = next_event;
                    waiting_event = true;
                    break;
                }
            }
        }

        // Trouve le widget concerné par l'événement
        ei_widget_t target = NULL;
        if (event.type == ei_ev_exposed && root_widget != NULL) {
            // Quand la fenêtre est exposée (il faut la redessiner), on redessine tout pour éviter les artefacts
            ei_app_invalidate_rect(&root_widget->screen_location);
        }
        // Si event est situé, target est non NULL
        if ((event.type == ei_ev_mouse_buttondown) || (event.type == ei_ev_mouse_buttonup) ||
            (event.type == ei_ev_mouse_move) || (event.type == ei_ev_mouse_wheel)) {
            target = ei_widget_pick(&event.param.mouse.where);
        }

        ei_search_event(&event, target);

        if (rects_todo != NULL) {
            // Pendant les rafales de mouvements de souris, on cadence l'affichage pour éviter le clignotement visuel.
            if (event.type == ei_ev_mouse_move) {
                double now = hw_now();
                if ((now - last_time) < min_frame_period) {
                    continue;
                }
            }

            hw_surface_lock(root_surface);
            hw_surface_lock(pick_surface);
            root_widget->wclass->drawfunc(root_widget, root_surface, pick_surface, &rects_todo->rect);
            hw_surface_unlock(pick_surface);
            hw_surface_unlock(root_surface);
            hw_surface_update_rects(root_surface, rects_todo);
            last_time = hw_now();

            ei_linked_rect_t* cur = rects_todo;
            while (cur != NULL) {
                ei_linked_rect_t* next = cur->next;
                free(cur);
                cur = next;
            }
            rects_todo = NULL;
        }
    }
}

void ei_app_invalidate_rect(const ei_rect_t* rect) {
    if (rect) {
        if (rect->size.width <= 0 || rect->size.height <= 0) return;

        // Clipper le rect à la surface root pour éviter les débordements
        ei_rect_t root_rect = hw_surface_get_rect(root_surface);
        ei_rect_t clipped = ei_rect_intersection(rect, &root_rect);

        if (clipped.size.width <= 0 || clipped.size.height <= 0) return;
        if (rects_todo == NULL) {
            ei_linked_rect_t* nouveau_rect_todo = malloc(sizeof(ei_linked_rect_t));
            nouveau_rect_todo->rect = *rect;
            nouveau_rect_todo->next = NULL;
            rects_todo = nouveau_rect_todo;
            return;
        }
        rects_todo->rect = rect_union(rects_todo->rect, *rect);
    }
}

void ei_app_quit_request()
{
    stop_condition = false;
}

ei_widget_t ei_app_root_widget(void) { return root_widget; }
ei_surface_t ei_app_root_surface(void) { return root_surface; }
ei_surface_t ei_app_pick_surface(void) { return pick_surface;}
ei_linked_rect_t* ei_app_rects_todo(void){return rects_todo;}