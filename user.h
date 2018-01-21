#ifndef _USER_H
#define _USER_H

/**
* \brief    Affiche le menu d'un resto avec les quantités disponibles au moment de l'appel.
* \param    stock       La mémoire partagée du stock du restaurant.
* \param    pid_resto   Le pid du resto duquel on affiche le menu.
*/
void afficher_menu(PLAT* stock, int pid_resto);

#endif
