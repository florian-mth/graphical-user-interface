#include "ei_widgetclass.h"
#include <string.h>
#include "ei_widgetclass.h"
#include "ei_implementation.h"

static ei_widgetclass_t* g_widget_class=NULL;

size_t ei_widget_struct_size(void) {//retourne la taille
    return sizeof(ei_impl_widget_t);
}

void ei_widgetclass_register(ei_widgetclass_t* widgetclass) { // ajoute un type de widget à la liste des widget
    widgetclass->next= g_widget_class;
    g_widget_class= widgetclass;
}

ei_widgetclass_t* ei_widgetclass_from_name(ei_const_string_t name) { // parcourt la liste chainée des widget et renvoie le type de widget correspondant au nom
    ei_widgetclass_t* cur = g_widget_class; // recup tête de liste
    while (cur != NULL) {
        if (strcmp(cur->name, name) == 0) { //regarde l'égalité avec le nom
            return cur; //renvoie la classe du widget
        }
        cur= cur->next;
    }
    return NULL;
}