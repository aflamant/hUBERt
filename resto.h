#ifndef _RESTO_H
#define _RESTO_H

/**
* \brief    Affiche le stock du resto avec les noms des produits et leurs quantités.
*/
void afficher_stock();

/**
* \brief    Initialise le stock du resto et envoie les informations de connexion à hubert.
* \param    type     Le type du resto.
* \return   0 si la fonction a réussi, -1 si elle a échoué.
*/
int enregistrer_resto(int type);


/**
* \brief    Ferme tous les IPCs créés par le resto, ainsi que le processus fils cuisine. Envoie à hubert un message de deconnexion.
*
*  Appelée par un signal d'interruption.
*/
void fermeture_resto() ;

/*
* \brief    Ne fait rien, sert uniquement à réveiller le processus cuisine d'une fonction pause()
*/
void sigusr1_handle();

#endif
