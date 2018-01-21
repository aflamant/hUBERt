#ifndef _HUBERT_H
#define _HUBERT_H

#define N_RESTOS 20
#define N_COURSIERS 3


/**
* \brief    Ajoute un resto à la liste des restos de hubert. Si le nombre maximal de restos est atteint, renvoie au resto un signal d'interruption.
* \param    pid         Le pid du resto à ajouter.
* \param    type        Le type du resto à ajouter.
*/
void ajouter_resto(int pid, int type);

/**
* \brief    Supprime un resto de la liste des restos de hubert, est appelé quand un processus resto est interrompu.
* \param    pid         Le pid du resto à supprimer.
*/
void supprimer_resto(int pid);

/**
* \brief    Renvoie le pid d'un resto enregistré correspondant au type voulu par l'utilisateur.
* \param    type        Le type du resto recherché
* \return   Le pid du premier resto de ce type trouvé dans la liste de hubert.
*/
int chercher_resto(int id_resto);

/**
* \brief    Ferme tous les IPCs créés par hubert, envoie un signal d'interruption à tous les restos connectés. Appelé quand hubert reçoit un signal d'interruption.
*/
void fermeture_hubert();

#endif
