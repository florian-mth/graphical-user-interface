#include "ei_types.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "widget_entry_class.h"
#include "widget_button_class.h"
#include "ei_draw.h"
#include "ei_application.h"
#include "ei_entry.h"
#include "ei_event.h"
#include "intersection.h"

static ei_entry_t* g_active_text_field = NULL;  // Pointeur vers le champ de saisie actif
double last_time = 0.0;
const double clignottement = 0.5;
static bool cursor_visible = true;

static bool entry_is_active(ei_entry_t* entry)
{
    return g_active_text_field == entry;
}

// Champ de saisie sans la bordure (rectangle colorié en blanc)
static ei_rect_t entry_inner_rect(ei_widget_t widget, bool input_active)
{
    ei_entry_t* entry = (ei_entry_t*)widget;
    int visible_bw = input_active ? entry->border_width : entry->border_width - 1;
    if (visible_bw < 1)
        visible_bw = 1;

    return (ei_rect_t) {
        .top_left = {
            widget->screen_location.top_left.x + visible_bw,
            widget->screen_location.top_left.y + visible_bw
        },
        .size = {
            widget->screen_location.size.width - 2 * visible_bw,
            widget->screen_location.size.height - 2 * visible_bw
        }
    };
}

// Calcule la largeur en pixels des n premiers caractères du texte
static int text_width_n(const char* text, ei_font_t font, int n)
{
    if (n <= 0 || text == NULL) return 0;
    int len = (int)strlen(text);
    if (n > len) n = len;

    char* buf = malloc((size_t)n + 1);
    if (!buf) return 0;
    memcpy(buf, text, (size_t)n);
    buf[n] = '\0';

    int tw = 0, th = 0;
    hw_text_compute_size(buf, font, &tw, &th);
    free(buf);
    return tw;
}

// Trouve l'index de curseur le plus proche du pixel click_x (coordonnée absolue)
static int cursor_pos_from_click(const char* text, ei_font_t font, int text_start_x, int click_x)
{
    if (text == NULL) return 0;
    int len = (int)strlen(text);
    if (len == 0) return 0;

    // Position 0 : avant le premier caractère
    int best_pos = 0;
    int best_dist = abs(click_x - text_start_x);

    // On teste les positions de 1 jusqu'à len (après chaque caractère)
    for (int i = 1; i <= len; i++) {
        int x = text_start_x + text_width_n(text, font, i);
        int dist = abs(click_x - x);
        if (dist < best_dist) {
            best_dist = dist;
            best_pos = i;
        }
    }
    return best_pos;
}

ei_widget_t entry_allocfunc(void)
{
    ei_entry_t* entry = calloc(1, sizeof(ei_entry_t));
    return (ei_widget_t)entry;
}

void entry_releasefunc(ei_widget_t widget)
{
    ei_entry_t* entry = (ei_entry_t*)widget;
    if (entry->text_input != NULL) {
        free(entry->text_input);
        entry->text_input = NULL;
    }
    if (g_active_text_field == entry)
        g_active_text_field = NULL;
}

void entry_setdefaultsfunc(ei_widget_t widget)
{
    ei_entry_t* entry = (ei_entry_t*) widget;
    entry->bg_color = (ei_color_t){0xff, 0xff, 0xff, 0xff};
    entry->border_width = 2;
    entry->border_color = (ei_color_t){0x70, 0x70, 0x70, 0xff};
    entry->text_input = NULL;
    entry->text_font = ei_default_font;
    entry->text_color = ei_font_default_color;
    entry->requested_char_size = 10;
    entry->callback = NULL;
    entry->user_param = NULL;
    entry->cursor_pos    = 0;
    entry->scroll_offset = 0;

    widget->requested_size = (ei_size_t){0, 0};
    widget->content_rect = &widget->screen_location;
}

void entry_drawfunc(ei_widget_t widget, ei_surface_t surface, ei_surface_t pick_surface, ei_rect_t* clipper)
{
    ei_entry_t* entry = (ei_entry_t*)widget;
    bool input_active = entry_is_active(entry);
    ei_rect_t outer = widget->screen_location;  // Champ de saisie avc la bordure
    ei_rect_t inner = entry_inner_rect(widget, input_active);  // Champ sans la bordure

    if (pick_surface != NULL) {
        ei_color_t pick_color = widget->pick_color;
        ei_polygone_t* pick_shape = rounded_frame(widget->screen_location,
                                                  0, whole, entry->border_width);
        pick_color.alpha = 0xff;
        ei_draw_polygon(pick_surface, pick_shape->tab, pick_shape->tab_size, pick_color, clipper);
        free(pick_shape->tab);
        free(pick_shape);
    }

    ei_fill(surface, &entry->border_color, &outer);
    ei_fill(surface, &entry->bg_color, &inner);

    const char* text = (entry->text_input != NULL) ? entry->text_input : "";
    int text_height = 0, text_width = 0;
    hw_text_compute_size(text, entry->text_font, &text_width, &text_height);

    // On définit un clipper pour que le texte ne sorte pas du champ de saisie
    ei_rect_t text_clipper = inner;
    if (clipper != NULL)
        text_clipper = ei_rect_intersection(&text_clipper, clipper);

    // Si le clipper est nul on ne dessine rien
    if (text_clipper.size.width == 0 || text_clipper.size.height == 0) {
        ei_impl_widget_draw_children(widget, surface, pick_surface, clipper);
        return;
    }

    int text_len = strlen(text);
    int available_width = inner.size.width - 8;

    if (entry->cursor_pos < entry->scroll_offset)
        entry->scroll_offset = entry->cursor_pos;

    // Si le texte est trop grand par rapport au champ de saisie, on le décale vers la gauche
    while (entry->scroll_offset < entry->cursor_pos)
    {
        int w, h;  // Largeur et hauteur du texte
        hw_text_compute_size(&text[entry->scroll_offset], entry->text_font, &w, &h);
        // On coupe le texte au niveau du caractère sur le curseur
        char car_saved = ((char*)text)[entry->cursor_pos];
        ((char*)text)[entry->cursor_pos] = '\0';
        int wc, hc;
        // wc est la longueur du texte entre l'offset et le curseur
        hw_text_compute_size(&text[entry->scroll_offset], entry->text_font, &wc, &hc);
        ((char*)text)[entry->cursor_pos] = car_saved;  // On remet le caractère
        // Si wc est trop grand par rapport au champ, on augmente l'offset
        // Sinon on sort de la boucle
        if (wc > available_width)  {
            entry->scroll_offset++;
        } else
        {
            break;
        }
    }

    // Texte visible depuis le scroll_offset
    const char* visible_text = &text[entry->scroll_offset];

    int tw, th;
    hw_text_compute_size(visible_text[0] ? visible_text : " ",
                         entry->text_font, &tw, &th);

    int text_y = inner.top_left.y + (inner.size.height - th) / 2;
    ei_point_t text_pos = {inner.top_left.x + 4, text_y};

    if (text_len > 0)
        ei_draw_text(surface, &text_pos, visible_text,
                     entry->text_font, entry->text_color, &text_clipper);
        // Si le champ est actif, on gère le curseur
        if (input_active) {

            int cursor_x_offset = 0;
            if (entry->cursor_pos > entry->scroll_offset) {
                char saved = ((char*)text)[entry->cursor_pos];
                ((char*)text)[entry->cursor_pos] = '\0';
                int cw, ch;
                // cw est la position du curseur par rapport au champ de saisie
                hw_text_compute_size(&text[entry->scroll_offset],
                                     entry->text_font, &cw, &ch);
                ((char*)text)[entry->cursor_pos] = saved;
                cursor_x_offset = cw;
            }

            // Mise à jour de la logique de clignotement
            double now = hw_now();
            if (now - last_time >= clignottement) {
                cursor_visible = !cursor_visible;
                last_time = now;
            }

            // Dessin du curseur seulement s'il est dans sa phase "visible"
            if (cursor_visible) {
                // Calcul de la position X du curseur basée sur cursor_pos
                ei_rect_t cursor_rect = {
                    .top_left = {
                        inner.top_left.x + 4 + cursor_x_offset,
                        inner.top_left.y + 3
                    },
                    .size = {1, inner.size.height - 6}
                };

                // On s'assure que le curseur est aussi clippé par le rectangle interne
                ei_rect_t cursor_clipper_final = ei_rect_intersection(&cursor_rect, &text_clipper);

                if (cursor_clipper_final.size.width > 0 && cursor_clipper_final.size.height > 0) {
                    ei_fill(surface, &entry->text_color, &cursor_clipper_final);
                }
            }
        }
    ei_impl_widget_draw_children(widget, surface, pick_surface, clipper);
}

void entry_geomnotifyfunc(ei_widget_t widget)
{
    widget->content_rect_storage = entry_inner_rect(widget, entry_is_active((ei_entry_t*)widget));
    widget->content_rect = &widget->content_rect_storage;
}

ei_widgetclass_t* entry_widgetclass(void)
{
    static ei_widgetclass_t klass = {
        "entry",
        entry_allocfunc,
        entry_releasefunc,
        entry_drawfunc,
        entry_setdefaultsfunc,
        entry_geomnotifyfunc,
        NULL
    };
    return &klass;
}

bool entry_on_click(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    last_time = hw_now();
    cursor_visible = true;

    (void)user_param;

    if (widget == NULL)
        return false;

    if (g_active_text_field != NULL)
        ei_app_invalidate_rect(&((ei_widget_t)g_active_text_field)->screen_location);

    g_active_text_field = (ei_entry_t*)widget;

    ei_entry_t* entry = (ei_entry_t*)widget;
    ei_rect_t inner = entry_inner_rect(widget, true);
    int text_x = inner.top_left.x + 4;
    int click_x = event->param.mouse.where.x;

    const char* text = entry->text_input ? entry->text_input : "";
    // permet d'eviter d'avoir des caracteres aleatoires dans l'entry
    if (text == "") {
        ei_entry_set_text(widget, text);
    }
    entry->cursor_pos = cursor_pos_from_click(text, entry->text_font, text_x, click_x);

    ei_app_invalidate_rect(&widget->screen_location);
    return true;
}

bool entry_on_text_input(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    (void)widget;
    (void)user_param;

    if (g_active_text_field == NULL)
        return false;

    ei_entry_t* entry = g_active_text_field;
    char c = event->param.text;
    int len = entry->text_input ? strlen(entry->text_input) : 0;   // len ne compte pas '\0'

    // Si requested_size est non-nulle et que le champ de saisie est déjà rempli au maximum, on n'ajoute pas 'c'
    if ((entry->requested_char_size) && (len >= entry->requested_char_size)) return true;

    char* new_text = realloc(entry->text_input, len + 2); // On ajoute 'c'
    if (new_text == NULL) return false;

    memmove(&new_text[entry->cursor_pos + 1], &new_text[entry->cursor_pos], len - entry->cursor_pos + 1);

    new_text[entry->cursor_pos] = c;
    entry->text_input = new_text;
    entry->cursor_pos++;

    ei_app_invalidate_rect(&((ei_widget_t)entry)->screen_location);
    return true;
}

bool entry_on_keydown(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    if (g_active_text_field == NULL) return false;

    ei_entry_t* entry = g_active_text_field;
    SDL_Keycode key = event->param.key_code;
    int len = entry->text_input ? (int)strlen(entry->text_input) : 0;

    // Si backspace, on supprime le caractère avant le curseur
    if (key == SDLK_BACKSPACE) {
        if (entry->text_input != NULL && entry->cursor_pos > 0) {
            int pos = entry->cursor_pos;
            memmove(entry->text_input + pos - 1,entry->text_input + pos, (size_t)(len - pos) + 1);
            entry->cursor_pos--;
            ei_app_invalidate_rect(&((ei_widget_t)entry)->screen_location);
        }
        return true;
    }

    // Si delete, on supprime le caractère après le curseur
    if (key == SDLK_DELETE) {
        if (entry->text_input != NULL && entry->cursor_pos < len) {
            int pos = entry->cursor_pos;
            memmove(entry->text_input + pos, entry->text_input + pos + 1, (size_t)(len - pos));
            ei_app_invalidate_rect(&((ei_widget_t)entry)->screen_location);
        }
        return true;
    }

    // Flèche gauche
    if (key == SDLK_LEFT) {
        if (entry->cursor_pos > 0) {
            entry->cursor_pos--;
            ei_app_invalidate_rect(&((ei_widget_t)entry)->screen_location);
        }
        return true;
    }

    // Flèche droite
    if (key == SDLK_RIGHT) {
        if (entry->cursor_pos < len) {
            entry->cursor_pos++;
            ei_app_invalidate_rect(&((ei_widget_t)entry)->screen_location);
        }
        return true;
    }

    // Si home, on va au début du texte
    if (key == SDLK_HOME) {
        if (entry->cursor_pos != 0) {
            entry->cursor_pos = 0;
            ei_app_invalidate_rect(&((ei_widget_t)entry)->screen_location);
        }
        return true;
    }

    // Si end, on va à la fin du texte
    if (key == SDLK_END) {
        if (entry->cursor_pos != len) {
            entry->cursor_pos = len;
            ei_app_invalidate_rect(&((ei_widget_t)entry)->screen_location);
        }
        return true;
    }

    // Si entrée, on valide le champ et on appelle son callback
    if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        if (entry->callback != NULL)
            entry->callback((ei_widget_t)entry, event, entry->user_param);
        g_active_text_field = NULL;
        ei_app_invalidate_rect(&((ei_widget_t)entry)->screen_location);
        return true;
    }

    // Si tab, on passe au champ entry suivant
    if (key == SDLK_TAB) {
        ei_widget_t current = (ei_widget_t)entry;
        ei_widget_t parent = current->parent;
        if (parent == NULL)
            return false;

        // On cherche le prochain widget de classe entry
        ei_widget_t sibling = current->next_sibling;
        while (sibling != NULL) {
            if (sibling->wclass != NULL && strcmp(sibling->wclass->name, "entry") == 0)
                break;
            sibling = sibling->next_sibling;
        }

        // Si sibling est nul, c'est qu'il est à la fin de la liste chainée
        if (sibling == NULL) {
            sibling = parent->children_head;
            // On re-parcourt la liste en partant du début
            while (sibling != NULL && sibling != current) {
                if (sibling->wclass != NULL && strcmp(sibling->wclass->name, "entry") == 0)
                    break;
                sibling = sibling->next_sibling;
            }
        }

        if (sibling != NULL && sibling != current) {
            ei_app_invalidate_rect(&current->screen_location);
            g_active_text_field = (ei_entry_t*)sibling;
            ei_entry_t* next_entry = (ei_entry_t*)sibling;
            next_entry->cursor_pos = next_entry->text_input ? (int)strlen(next_entry->text_input) : 0;
            ei_app_invalidate_rect(&sibling->screen_location);
            return true;
        }
    }
    return false;
}

void focus_on_entry(ei_widget_t widget)
{
    if (widget == NULL) return;

    if (g_active_text_field != NULL)
        ei_app_invalidate_rect(&((ei_widget_t)g_active_text_field)->screen_location);

    g_active_text_field = (ei_entry_t*)widget;
    ei_entry_t* entry = (ei_entry_t*)widget;
    entry->cursor_pos = entry->text_input ? (int)strlen(entry->text_input) : 0;
    ei_app_invalidate_rect(&widget->screen_location);
}

bool entry_blink_handler(ei_widget_t widget, ei_event_t* event, void* user_param)
{
    (void)widget;
    (void)event;
    (void)user_param;

    if (g_active_text_field != NULL) {
        ei_app_invalidate_rect(
            &((ei_widget_t)g_active_text_field)->screen_location
        );
    }
    return false;
}