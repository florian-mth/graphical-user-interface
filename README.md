# <u>Projet C : Interface Graphique</u>

Trinôme : Mathieu Florian, Ringuede Lucien, Boussahla Walid

## Table des matières :

- [Nos tests](#nos-tests-)

- [Les tests fournis dans le sujet](#les-tests-fournis-dans-le-sujet-)

## Nos tests :

- ###### `button_transparent.c` :

Permet de tester la transparence des boutons (un choix d'implémentation que nous avons fait), la gestion des évènements (bind et unbind, callback), l'implémentation de pick_surface et la gestion du relief

- ###### `frame_transparent.c` :

Confirmer l'implémentation de la transparence pour une frame et aussi tester le gestionnaire de géométrie (fonction ei_place)

- ###### `hello_world_parenting_fail.c` :

Test "d'inclusion" d'une toplevel dans une toplevel et des conséquences sur le drag et le resize.

- ###### `blackjack.c` :

Petit jeu bonus pour tester l'import d'image, la suppression d'image (les cartes), l'entry, les boutons avec des callback mais aussi pour voir comment faire un jeu fonctionnel avec la bibliothèque.

## Les tests fournis dans le sujet :

- ###### `two048.c` :

Lorsqu'on clique sur le header d'une toplevel, celle-ci est mise au premier plan. Gestion de l'affichage à chaque fois qu'une touche du clavier est appuyée.

- ###### `entry.c` :

Clignotement du curseur, surbrillance de la case d'entrée lorsqu'elle est sélectionné, gestion des touches tab, début et fin, gestion du scroll du text et du déplacement avec les flèches, gestion du texte affiché par rapport au texte total

- ###### `ext_testclass.c` :

Gestion de la nouvelle classe notamment avec les flèches du clavier et la souris.

- ###### `frame.c` :

Bordures de la frame et écriture du texte à l'intérieur.

- ###### `button.c` :

Idem à button_transparent, gestion du clipper

- ###### `Hello_world` :

Test des deux boutons associés à une toplevel : size_button et close_button. Gestion des ancres dans ei_place et vérification de la transparence d'une toplevel (header et body dissociés).

Optimisation de la fonction ei_app_run() car surcharge du processeur à cause des drag et resize.

- ###### `minesweeper.c` :

Utilisation des images sur des frames


- ###### `puzzle.c` :

Utilisation des images sur des boutons
