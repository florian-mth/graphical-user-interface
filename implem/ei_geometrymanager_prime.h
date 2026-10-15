//
// Created by ringuede on 04/05/2026.
//

#ifndef PROJETC_IG_EI_GEOMETRYMANAGER_PRIME_H
#define PROJETC_IG_EI_GEOMETRYMANAGER_PRIME_H

#include "ei_geometrymanager.h"
#include "ei_types.h"

ei_rect_t ei_place_anchor_in_rect(ei_rect_t container, ei_size_t size, ei_anchor_t anchor);

void ei_placer_runfunc(ei_widget_t widget);
static void placer_releasefunc(ei_widget_t widget);
ei_geometrymanager_t* placer_widgetclass(void);


#endif //PROJETC_IG_EI_GEOMETRYMANAGER_PRIME_H
