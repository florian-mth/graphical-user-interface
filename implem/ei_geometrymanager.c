#include "ei_geometrymanager.h"

#include "ei_application.h"
#include "ei_implementation.h"
#include "ei_utils.h"
static ei_geometrymanager_t* geo_manager=NULL; // pointeur vers la tete des noms des managers

size_t ei_geom_param_size(void) {
    return sizeof(ei_impl_geom_param_t);
}

void ei_geometrymanager_register(ei_geometrymanager_t* geometrymanager)
{
    geometrymanager->next = geo_manager; // ajoute une case
    geo_manager = geometrymanager; // la case ajoutée est le nouveau geometrymanager
}

ei_geometrymanager_t* ei_geometrymanager_from_name (ei_geometrymanager_name_t name){
    ei_geometrymanager_t* cur = geo_manager; // recup la tête de liste
    while (cur != NULL) {
        if (strcmp(cur->name, name) == 0) { // si ils ont le même nom, on a trouvé
            return cur; // on renvoie la classe
        }
        cur= cur->next;
    }
    return NULL;
}

void ei_geometrymanager_unmap (ei_widget_t widget) {
    if (widget==NULL || widget->geom_params==NULL) { // si le widget est null ou ses paramètres null, on ne fait rien
        return;
    }
    ei_geometrymanager_t *manager = ei_widget_get_geom_manager(widget); // récupère le manager du widget
    if (manager!=NULL && manager->releasefunc!=NULL) { // si il est non null et que sa releasefunc est implementée on le release
        manager->releasefunc(widget);
    }
    free(widget->geom_params); // on free les paramètres
    widget->geom_params=NULL; // met à null
    ei_app_invalidate_rect(&widget->screen_location);
    widget->screen_location=ei_rect_zero(); // met la screen location à zero
}

ei_geometrymanager_t* ei_widget_get_geom_manager (ei_widget_t widget) {
    if (widget==NULL || widget->geom_params==NULL) {
        return NULL;
    }
    ei_impl_geom_param_t *params = widget->geom_params;
    return params->manager;
}

void ei_widget_set_geom_manager	(ei_widget_t widget, ei_geometrymanager_t* manager) {
    if (widget==NULL || widget->geom_params==NULL) {
        return;
    }
    ei_impl_geom_param_t *params = widget->geom_params;
    params->manager=manager;
}

ei_geom_param_t	ei_widget_get_geom_params (ei_widget_t widget) {
    if (widget==NULL) { //si null on passe
        return NULL;
    }
    return widget->geom_params; // récupère geom_params
}

void ei_widget_set_geom_params (ei_widget_t widget, ei_geom_param_t geom_param) {
    // même chose que get_geom_params
    if (widget==NULL) {
        return;
    }
    widget->geom_params = geom_param;
}

void eip_geometrymanager_finalize (ei_widget_t widget, ei_rect_t* new_screen_location){
    // regarde si le widget a changé de place, si oui on met à jour sa screen_location, on notify l'instance du widget si possible puis on met à jour les enfants
    if (widget == NULL || new_screen_location==NULL) { // si null on passe
        return;
    }
    bool changed= memcmp(&widget->screen_location, new_screen_location,sizeof(ei_rect_t)) != 0;  // regarde si on a changé de place
    if (!changed) {
        return;
    }
    ei_app_invalidate_rect(&widget->screen_location);
    widget->screen_location = *new_screen_location; // met à jour si on a changé
    ei_app_invalidate_rect(&widget->screen_location);
    if (widget->wclass != NULL && widget->wclass->geomnotifyfunc!=NULL) { // notify si possible
        widget->wclass->geomnotifyfunc(widget);
    }
    ei_widget_t child = widget->children_head; // récupère la liste chainée des enfants
    while (child!=NULL) {
        ei_geometrymanager_t* child_manager = ei_widget_get_geom_manager(child);  //recup le manager
        if (child_manager!=NULL && child_manager->runfunc!=NULL) { //runfunc si possible
            child_manager->runfunc(child);
        }
        child=child->next_sibling;
    }
}