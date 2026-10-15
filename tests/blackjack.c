#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ei_application.h"
#include "ei_widget.h"
#include "ei_utils.h"
#include "ei_event.h"
#include "ei_widget_configure.h"
#include "hw_interface.h"
#include "ei_placer.h"
#include "ei_entry.h"
#include "ei_widget_attributes.h"
typedef struct {
    int player_money;
    int current_bet;
    int player_score;
    int dealer_score;
    int card_count_player;
    int card_count_dealer;
    char* deck[52];
    int next_card_index;
    bool standing;
    bool dealer_playing;

    ei_widget_t root;
    ei_widget_t money_label;
    ei_widget_t entry_bet;
    ei_widget_t player_hand_frame;
    ei_widget_t dealer_hand_frame;
    ei_widget_t score_player_label;
    ei_widget_t score_dealer_label;
    ei_widget_t status_label;
} blackjack_t;


void update_money_display(blackjack_t* game) {
    char buf[32];
    sprintf(buf, "Argent: %d$", game->player_money);
    ei_string_t text = buf;
    ei_frame_configure(game->money_label, NULL, NULL, NULL, NULL, &text, NULL, NULL, NULL, NULL, NULL, NULL);
}

void update_scores_display(blackjack_t* game) {
    char p_buf[32], d_buf[32];
    sprintf(p_buf, "Joueur: %d", game->player_score);
    sprintf(d_buf, "Dealer: %d", game->dealer_score);
    ei_string_t p_txt = p_buf;
    ei_string_t d_txt = d_buf;
    ei_frame_configure(game->score_player_label, NULL, NULL, NULL, NULL, &p_txt, NULL, NULL, NULL, NULL, NULL, NULL);
    ei_frame_configure(game->score_dealer_label, NULL, NULL, NULL, NULL, &d_txt, NULL, NULL, NULL, NULL, NULL, NULL);
}

void set_status(blackjack_t* game, const char* msg, ei_color_t color) {
    ei_string_t txt = (ei_string_t)msg;
    ei_frame_configure(game->status_label, NULL, NULL, NULL, NULL, &txt, NULL, &color, NULL, NULL, NULL, NULL);
}

void add_card(ei_widget_t parent, const char* card_name, int pos) {
    ei_widget_t card = ei_widget_create("frame", parent, NULL, NULL);
    char path[128];
    sprintf(path, "misc/cartes-gif/%s.png", card_name);
    ei_surface_t img = hw_image_load(path, ei_app_root_surface());
    // on met l'image (si erreur dans le chargement on met un carré rouge)
    if (img == NULL) {
        ei_color_t red = {255, 0, 0, 255};
        ei_frame_configure(card, &(ei_size_t){40, 60}, &red, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    } else {
        ei_size_t img_size = hw_surface_get_size(img);
        ei_frame_configure(card, &img_size, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &img, NULL, NULL);
    }
    int x = pos * 75;
    int y = 10;
    ei_place(card, NULL, &x, &y, NULL, NULL, NULL, NULL, NULL, NULL);
}


int get_card_value(const char* card_name, int current_score) {
    if (card_name[0] == '1' || card_name[0] == 'J' || card_name[0] == 'Q' || card_name[0] == 'K') return 10;
    if (card_name[0] == 'A') {
        return (current_score + 11 <= 21) ? 11 : 1; // si c'est un as et que on depasse 21 avec 11 alors l'as vaut 1
    }
    return card_name[0] - '0';
}

void shuffle_deck(blackjack_t* game) {
    //melange le deck
    for (int i = 51; i > 0; i--) {
        int j = rand() % (i + 1);
        char* temp = game->deck[i];
        game->deck[i] = game->deck[j];
        game->deck[j] = temp;
    }
    game->next_card_index = 0;
}

void init_deck(blackjack_t* game) {
    // initialise le deck de carte
    const char* values[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
    const char* suits[] = {"h", "s", "d", "c"};
    int k = 0;
    for (int s = 0; s < 4; s++) {
        for (int v = 0; v < 13; v++) {
            game->deck[k] = malloc(16);
            sprintf(game->deck[k], "%s%s", values[v], suits[s]);
            k++;
        }
    }
    shuffle_deck(game);
}


bool bet_callback(ei_widget_t widget, ei_event_t* event, void* user_param) {
    // On relance une partie avec un nouveau bet, on efface tout et on tire 3 cartes(deux pour le joueur, une pour le dealer)
    blackjack_t* game = (blackjack_t*)user_param;
    const char* bet_str = ei_entry_get_text(game->entry_bet);
    int bet = (bet_str) ? atoi(bet_str) : 0;

    if (bet <= 0) {
        set_status(game, "Mise invalide !", (ei_color_t){255, 200, 0, 255});
        return true;
    }
    if (bet > game->player_money) {
        set_status(game, "Mise trop haute !", (ei_color_t){255, 100, 0, 255});
        return true;
    }
    // mets a jour les vars
    game->current_bet = bet;
    game->player_money -= bet;
    game->player_score = 0;
    game->dealer_score = 0;
    game->card_count_player = 0;
    game->card_count_dealer = 0;
    game->standing = false;

    // Detruit les anciennes cartes
    ei_widget_t child;
    while ((child = ei_widget_get_first_child(game->player_hand_frame))!=NULL)
        ei_widget_destroy(child);
    while ((child = ei_widget_get_first_child(game->dealer_hand_frame))!=NULL)
        ei_widget_destroy(child);

    set_status(game, "Bonne chance !", (ei_color_t){255, 255, 255, 255});
    update_money_display(game);
    // on melange si on a moins de 3 cartes de libres
    if (game->next_card_index >= 49) {
        set_status(game, "Deck melangé", (ei_color_t){255, 215, 0, 255});
        shuffle_deck(game);
    }
    // Tirage initial : 2 cartes joueur, 1 carte dealer (classique blackjack)
    char* card;

    card = game->deck[game->next_card_index++];
    game->player_score += get_card_value(card, game->player_score);
    add_card(game->player_hand_frame, card, game->card_count_player++);

    card = game->deck[game->next_card_index++];
    game->dealer_score += get_card_value(card, game->dealer_score);
    add_card(game->dealer_hand_frame, card, game->card_count_dealer++);

    card = game->deck[game->next_card_index++];
    game->player_score += get_card_value(card, game->player_score);
    add_card(game->player_hand_frame, card, game->card_count_player++);

    update_scores_display(game);

    // Vérifie blackjack naturel
    if (game->player_score == 21) {
        set_status(game, "BLACKJACK !", (ei_color_t){255, 215, 0, 255});
        game->player_money += game->current_bet * 2;
        update_money_display(game);
        game->current_bet = 0;
    }

    return true;
}

bool hit_callback(ei_widget_t widget, ei_event_t* event, void* user_param) {
    // Tire une carte pour le joueur, la dessine, met a jour le score et regarde si on a gagné
    blackjack_t* game = (blackjack_t*)user_param;
    if (game->standing || game->current_bet == 0 || game->player_score >= 21) return false;

    if (game->next_card_index >= 52) {
        shuffle_deck(game);
        set_status(game, "Deck melangé", (ei_color_t){255, 215, 0, 255});
    }

    char* card = game->deck[game->next_card_index++];
    game->player_score += get_card_value(card, game->player_score);
    add_card(game->player_hand_frame, card, game->card_count_player++);

    update_scores_display(game);

    if (game->player_score > 21) {
        set_status(game, "BUST ! Perdu.", (ei_color_t){255, 0, 0, 255});
        game->current_bet = 0;
    } else if (game->player_score == 21) {
        set_status(game, "21 ! Faites STAND.", (ei_color_t){255, 215, 0, 255});
    }
    return true;
}
void finalize_game(blackjack_t* game) {
    // regarde le score du joueur et du dealer pour voir qui a gagné
    if (game->player_score <= 21 && (game->dealer_score > 21 || game->player_score > game->dealer_score)) {
        game->player_money += game->current_bet * 2;
        set_status(game, "GAGNE !", (ei_color_t){0, 255, 0, 255});
    } else if (game->player_score == game->dealer_score) {
        game->player_money += game->current_bet;
        set_status(game, "EGALITE", (ei_color_t){255, 255, 0, 255});
    } else {
        set_status(game, "PERDU", (ei_color_t){255, 0, 0, 255});
    }

    game->current_bet = 0;
    update_money_display(game);

    if (game->player_money <= 0) {
        set_status(game, "Vous etes ruine !", (ei_color_t){255, 0, 0, 255});
    }
}

bool dealer_step_callback(ei_widget_t widget, ei_event_t* event, void* user_param) {
    blackjack_t* game = (blackjack_t*)user_param;
    if (!game->dealer_playing) return true;

    // Tire une carte pour le dealer si besoin
    if (game->dealer_score < 17) {
        if (game->next_card_index >= 52) {
            shuffle_deck(game);
            set_status(game, "Deck melangé", (ei_color_t){255, 215, 0, 255});
        }
        char* card = game->deck[game->next_card_index++];
        game->dealer_score += get_card_value(card, game->dealer_score);
        add_card(game->dealer_hand_frame, card, game->card_count_dealer++);
        update_scores_display(game);

        // Invalide l'écran et schedule le prochain tirage(pour avoir une pose dans le tirage)
        ei_app_invalidate_rect(ei_widget_get_screen_location(game->root));
        hw_event_schedule_app(600, game);
    } else {
        // Le dealer a fini de tirer
        game->dealer_playing = false;
        finalize_game(game);
        ei_app_invalidate_rect(ei_widget_get_screen_location(game->root));
    }

    return true;
}

bool stand_callback(ei_widget_t widget, ei_event_t* event, void* user_param) {
    blackjack_t* game = (blackjack_t*)user_param;
    if (game->standing || game->current_bet == 0) return false;

    game->standing = true;
    game->dealer_playing = true;
    set_status(game, "Le dealer joue...", (ei_color_t){255, 255, 255, 255});

    // Lance le premier tirage après 600ms
    hw_event_schedule_app(600, game);
    return true;
}

void init_blackjack(blackjack_t* game) {
    // initialise toute les valeurs de jeu et toute les zones (frames, buttons, entry)
    ei_size_t win_size  = {800, 600};
    ei_color_t table_color = {0, 100, 0, 255};       // vert foncé
    ei_color_t dark_green  = {0, 80, 0, 255};        // vert encore plus foncé pour les frames
    ei_color_t white       = {255, 255, 255, 255};
    ei_color_t yellow      = {255, 215, 0, 255};

    init_deck(game);
    game->player_money = 100;
    game->current_bet  = 0;
    game->player_score = 0;
    game->dealer_score = 0;
    game->card_count_player = 0;
    game->card_count_dealer = 0;
    game->standing = false;
    game->next_card_index = 0;
    //root
    game->root = ei_app_root_widget();
    ei_frame_configure(game->root, &win_size, &table_color, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    //argent du joueur
    game->money_label = ei_widget_create("frame", game->root, NULL, NULL);
    ei_frame_configure(game->money_label, &(ei_size_t){160, 30}, &table_color,NULL, NULL, NULL, NULL, &yellow, NULL, NULL, NULL, NULL);
    int money_x = 0, money_y = 10;
    ei_place(game->money_label, NULL, &money_x, &money_y, NULL, NULL, NULL, NULL, NULL, NULL);
    update_money_display(game);

    // Champ de saisie de la mise
    game->entry_bet = ei_widget_create("entry", game->root, NULL, NULL);
    int taille_charactere = 5;
    int entry_x = 240, entry_y = 10;
    ei_entry_configure(game->entry_bet, &taille_charactere, &white, NULL, NULL, NULL);
    ei_place(game->entry_bet, NULL, &entry_x, &entry_y, NULL, NULL, NULL, NULL, NULL, NULL);

    static blackjack_t* p_game;
    p_game = game;

    // Bouton MISER
    ei_widget_t btn_bet = ei_widget_create("button", game->root, NULL, NULL);
    ei_string_t t_miser = "MISER";
    ei_callback_t cb_bet = bet_callback;
    ei_button_configure(btn_bet, &(ei_size_t){80, 40}, NULL, NULL, NULL, NULL,&t_miser, NULL, NULL, NULL, NULL, NULL, NULL,&cb_bet, (void**)&p_game);
    int button_x_bet = 345, button_y_bet = 10;
    ei_place(btn_bet, NULL, &button_x_bet, &button_y_bet, NULL, NULL, NULL, NULL, NULL, NULL);

    // Label Mise
    ei_widget_t mise_label = ei_widget_create("frame", game->root, NULL, NULL);
    ei_string_t txt_mise = "Mise:";
    ei_frame_configure(mise_label, &(ei_size_t){50, 30}, &table_color,NULL, NULL, &txt_mise, NULL, &white, NULL, NULL, NULL, NULL);
    int label_x = 190, label_y = 10;
    ei_place(mise_label, NULL, &label_x, &label_y, NULL, NULL, NULL, NULL, NULL, NULL);
    // label du score du dealer
    game->score_dealer_label = ei_widget_create("frame", game->root, NULL, NULL);
    ei_frame_configure(game->score_dealer_label, &(ei_size_t){160, 28}, &dark_green,NULL, NULL, NULL, NULL, &white, NULL, NULL, NULL, NULL);
    ei_anchor_t mid = ei_anc_center;
    float rx = 0.5f; // utilisé pour placer au milieu
    float rydealer_score = 0.11f;
    ei_place(game->score_dealer_label, &mid, NULL, NULL, NULL, NULL, &rx, &rydealer_score, NULL, NULL);
    // zone pour la main du dealer
    ei_size_t hand_sz = {600, 120};
    game->dealer_hand_frame = ei_widget_create("frame", game->root, NULL, NULL);
    ei_frame_configure(game->dealer_hand_frame, &hand_sz, &dark_green,NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    float ry_dealer_hand = 0.24f;
    ei_place(game->dealer_hand_frame, &mid, NULL, NULL, NULL, NULL, &rx, &ry_dealer_hand, NULL, NULL);
    //label pour le status du jeu
    game->status_label = ei_widget_create("frame", game->root, NULL, NULL);
    ei_string_t txt_status = "Entrez une mise et cliquez MISER";
    ei_frame_configure(game->status_label, &(ei_size_t){400, 34}, &table_color,NULL, NULL, &txt_status, NULL, &yellow, NULL, NULL, NULL, NULL);
    float ry_status = 0.43f;
    ei_place(game->status_label, &mid, NULL, NULL, NULL, NULL, &rx, &ry_status, NULL, NULL);
    // zone pour la main du joueur
    game->player_hand_frame = ei_widget_create("frame", game->root, NULL, NULL);
    ei_frame_configure(game->player_hand_frame, &hand_sz, &dark_green,
                       NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    float ry_player_hand = 0.62f;
    ei_place(game->player_hand_frame, &mid, NULL, NULL, NULL, NULL, &rx, &ry_player_hand, NULL, NULL);
    // score du joueur
    game->score_player_label = ei_widget_create("frame", game->root, NULL, NULL);
    ei_frame_configure(game->score_player_label, &(ei_size_t){160, 28}, &dark_green, NULL, NULL, NULL, NULL, &white, NULL, NULL, NULL, NULL);
    float ry_player_score = 0.76f;
    ei_place(game->score_player_label, &mid, NULL, NULL, NULL, NULL, &rx, &ry_player_score, NULL, NULL);

    ei_size_t btn_sz = {120, 44};// taille button
    float buttons_y = 0.95f; // y pour les boutons de hit et de stand

    // Bouton HIT
    ei_widget_t btn_hit = ei_widget_create("button", game->root, NULL, NULL);
    ei_string_t txt_hit = "HIT";
    ei_callback_t hit_cb = hit_callback;
    ei_button_configure(btn_hit, &btn_sz, &(ei_color_t){30, 144, 255, 255},NULL, NULL, NULL,&txt_hit, NULL, &white, NULL, NULL, NULL, NULL,&hit_cb, (void**)&p_game);
    float button_x_hit = 0.38f;
    ei_place(btn_hit, &mid, NULL, NULL, NULL, NULL, &button_x_hit, &buttons_y, NULL, NULL);

    // Bouton STAND
    ei_widget_t btn_std = ei_widget_create("button", game->root, NULL, NULL);
    ei_string_t txt_std = "STAND";
    ei_callback_t std_cb = stand_callback;
    ei_bind(ei_ev_app, NULL, "all", dealer_step_callback, game);
    ei_button_configure(btn_std, &btn_sz, &(ei_color_t){220, 50, 50, 255},NULL, NULL, NULL,&txt_std, NULL, &white, NULL, NULL, NULL, NULL,&std_cb, (void**)&p_game);
    float button_x_stand = 0.62f;
    ei_place(btn_std, &mid, NULL, NULL, NULL, NULL, &button_x_stand, &buttons_y, NULL, NULL);
    // met les scores a vide
    ei_string_t empty = " ";
    ei_frame_configure(game->score_player_label, NULL, NULL, NULL, NULL, &empty, NULL, NULL, NULL, NULL, NULL, NULL);
    ei_frame_configure(game->score_dealer_label, NULL, NULL, NULL, NULL, &empty, NULL, NULL, NULL, NULL, NULL, NULL);
}

int main(int argc, char** argv) {
    blackjack_t game;
    srand(time(NULL));
    ei_app_create((ei_size_t){800, 600}, false);
    init_blackjack(&game);
    ei_app_run();
    ei_app_free();
    return 0;
}