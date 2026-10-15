#include "ei_event.h"

typedef struct actions_t
{
    ei_eventtype_t eventtype;
    ei_widget_t widget;
    ei_tag_t tag;
    ei_callback_t callback;
    void* user_param;
    struct actions_t* next;
} actions_t;

void ei_search_event(ei_event_t* event, ei_widget_t widget);
void ei_widget_unbind_all(ei_widget_t widget);