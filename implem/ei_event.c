#include "ei_event.h"
#include "ei_event_prime.h"
#include "ei_implementation.h"

actions_t* list_actions = NULL;

// Ajoute une cellule (évènement, widet/tag, callback, user_param) en tête de la liste d'actions
void ei_bind (ei_eventtype_t eventtype, ei_widget_t widget, ei_tag_t tag,
        ei_callback_t callback, void* user_param)
{
    actions_t*	cell = calloc(1, sizeof(actions_t));
    cell->eventtype = eventtype;
    cell->widget = widget;
    cell->tag = tag;
    cell->callback = callback;
    cell->user_param = user_param;
    cell->next = list_actions;
    list_actions = cell;
}

// Enlève la cellule de la liste d'actions
void ei_unbind (ei_eventtype_t eventtype, ei_widget_t widget, ei_tag_t tag,
        ei_callback_t callback, void* user_param)
{
    actions_t* prev = NULL;
    actions_t* curr = list_actions;
    // On parcourt les éléments de la liste pour chercher celui qu'on doit enlever
    while (curr != NULL)
    {
        if ((curr->eventtype == eventtype) && ((curr->tag == NULL && tag == NULL) ||
            (curr->tag != NULL && tag != NULL && strcmp(curr->tag, tag) == 0)) &&
            (curr->callback == callback) && (curr->widget == widget && curr->user_param == user_param)) {
            if (prev == NULL)
            {
                list_actions = curr->next;
                free(curr);
                return;
            }
            // Sinon prev est déjà un élément de list_actions
            prev->next = curr->next;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void ei_search_event(ei_event_t* event, ei_widget_t widget) {
    actions_t* curr = list_actions;

    while (curr != NULL) {
        if (curr->eventtype == event->type) {
            bool match = false;

            if (curr->widget != NULL) {
                if (curr->widget == widget){
                    match = true;
                }
            } else if (curr->tag != NULL) {
                // Si tag == all on prend tous les widgets
                if (strcmp(curr->tag, "all") == 0) {
                    match = true;
                } else if (widget != NULL) {
                    // Sinon on prend les widget dont la classe correspond au tag
                    int val = strcmp(widget->wclass->name, curr->tag);
                    if (val == 0)
                    {
                        match = true;
                    }
                }
            }

            if (match) {
                bool targeted_widget = curr->callback(widget, event, curr->user_param);
                if (targeted_widget)
                {
                    return;  // Si callback renvoie true, on arrête
                            // Sinon, on continue de parcourir la liste
                }
            }
        }
        curr = curr->next;
    }
}

void ei_widget_unbind_all(ei_widget_t widget) {
    // unbind les events lié a widget
    actions_t* prev=NULL;
    actions_t* curr = list_actions;
    while (curr!=NULL) {
        if (curr->widget == widget) {
            actions_t* supp=curr;
            if (prev==NULL) {
                list_actions = curr->next;
                curr = list_actions;
            }else {
                prev->next = curr->next;
                curr = prev->next;
            }
            free(supp);
        }else {
            prev=curr;
            curr=curr->next;
        }
    }
}