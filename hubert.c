#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "hubert_types.h"
#include "hubert.h"

#include "semaphore.h"

/* ******************************************
*     Déclaration des variables globales.
* ******************************************/
int coursiers_id, msg_id;
int* index_restos;
int** restos;


int main(){

   signal(SIGINT, fermeture_hubert); /* Redirection du signal d'interruption vers notre fonction de fermeture du programme. */


   /* ********************************************************
   *    Initialisation et allocation du tableau des restos.
   * ********************************************************/
   restos = (int**)malloc(N_RESTOS*sizeof(int*)); //pointeur vers tableau 2d de restos avec les pid et les types
   int i;
   for (i = 0; i< N_RESTOS; i++) {
      restos[i] = (int*)malloc(2*sizeof(int));
   }


   /* ***************************************
   *     Création de la file de messages
   * ***************************************/
   key_t clef_message = ftok("hubert.c", 0);
   if (clef_message == -1) {
      printf("Erreur de création clé de la chaîne de messages.\n" );
      exit(-1);
   } //else printf("Clef chaîne de messages générée avec succès. = %d\n", clef_message);

   msg_id = msgget(clef_message, 0666 | IPC_CREAT);
   if (msg_id == -1) {
      printf("Erreur de création de la chaîne de messages.\n" );
      exit(-1);
   } //else printf("Chaîne de message créée. id = %d\n", msg_id);

   MSG msg_buf;


   /* ***************************************
   *     Création du sémaphore coursiers
   * ***************************************/
   key_t clef_coursiers = ftok("hubert.c", 1);
   if (clef_coursiers == -1) {
      printf("Erreur de création clé des coursiers.\n" );
      exit(-1);
   } //else printf("Clef coursiers générée avec succès. = %d\n", clef_coursiers);

   coursiers_id = create_semaphore(clef_coursiers);
   if (coursiers_id == -1) {
      printf("Erreur de création du sémaphore coursiers.\n" );
      exit(-1);
   } //else printf("Semaphore coursiers créé. id = %d\n", coursiers_id);

   init_semaphore(coursiers_id,N_COURSIERS);




   /* *************************************************
   *       Lancement du comportement de hubert.
   * *************************************************/
   printf("Initialistation de hUBERt.\n");
   printf("Nombre de coursiers présents aujourd'hui : %d\n", N_COURSIERS);

   while(1){

      msgrcv(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 1, 0); /* On attend la reception d'un message à destination de hubert */

      int pid_resto;
      int i;

      switch (msg_buf.type) { /*suivant le type de message reçu, on effectue différentes actions */

         /* Reception d'un message de déconnexion d'un restaurant */
         case -1 :
            supprimer_resto(msg_buf.signature);
            break;

         /* Reception d'un message de connexion d'un restaurant */
         case 0 :
            ajouter_resto(msg_buf.signature, msg_buf.id_resto);
            printf("Resto de type %d ajouté à hubert : %d\n", msg_buf.id_resto, msg_buf.signature);
            break;

         /* Reception d'un message de demande d'offre de la part d'un utilisateur. */
         case 1 :
            printf("Demande d'offre de restaurant de type %d la part de %d.\n", msg_buf.id_resto, msg_buf.signature );
            pid_resto = chercher_resto(msg_buf.id_resto); /* On cherche un resto correspondant à sa demande */
            printf("Envoi de la carte de %d\n", pid_resto );
            msg_buf.mtype = msg_buf.signature;
            msg_buf.signature = 0;
            msg_buf.type = 0;
            msg_buf.id_resto = pid_resto;
            msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0); /* On renvoie l'accès à la mémoire partagée de ce resto. */
            break;

         /* Reception d'une commande d'un utilisateur vers un resto */
         case 2 :
            printf("Commande de la part de l'utilisateur %d vers le restaurant %d\n", msg_buf.signature, msg_buf.id_resto );
            msg_buf.mtype = msg_buf.id_resto;
            msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0); /* On modifie uniquemement le destinataire du message pour le relayer au resto. */
            break;

         /* Reception d'un message de notification de l'envoi d'une commande depuis un resto vers un utilisateur. */
         case 3 :
            printf("La commande de %d pour %d vient d'être envoyée.\n", msg_buf.signature, msg_buf.id_resto );
            msg_buf.mtype = msg_buf.id_resto;
            msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0); /* On modifie uniquemement le destinataire du message pour le relayer à l'utilisateur. */
            break;
      }
   }
}

void ajouter_resto(int pid, int type){

   int i = 0;

   while (i<N_RESTOS && restos[i][0] != 0) { /* On parcoure le tableau jusqu'à trouver une case libre */
      i++;
   }

   if (i < N_RESTOS ) {       /* si on trouve une case vide on y inscrit le resto */
      restos[i][0] = pid;
      restos[i][1] = type;
   } else kill(pid,SIGINT);   /* sinon on envoie un signal d'interruption au resto qui essaie de se connecter */
}

void supprimer_resto(int pid) {

   int i =0;

   while(i<N_RESTOS && restos[i][0]!=pid) { /* On parcoure le tableau jusqu'à trouver le resto correspondant */
      i++;
   }

   if (i<N_RESTOS) {
      restos[i][0] = 0;
      restos[i][1] = 0;
      printf("Resto %d supprimé.\n",pid );
   }
}

int chercher_resto(int type){
   int i=0;

   while (i < N_RESTOS){
      if (restos[i][1] == type){
         return restos[i][0];       /* si l'on trouve un resto correspondant au type voulu on renvoie son pid */
      }
      i++;
   }

   return -1;                       /* si aucun resto ne correspond, on renvoie -1 */
}

void fermeture_hubert() {

   int i;

   for (i = 0; i<N_RESTOS; i++) {
      if (restos[i][0]!=0)
         kill(restos[i][0],SIGINT); /* on commence par envoyer un signal d'interruption à tous les restos connectés */
   }

   int res = msgctl (msg_id, IPC_RMID, NULL);  /* ensuite supression de la file de messages */
   if (res == -1) {
     printf("Erreur suppression file de messages.\n");
   } else {
     printf("File de messages supprimée.\n");
   }

   res = remove_semaphore(coursiers_id);        /* suppression du sémaphore coursiers */
   if (res == -1) {
     printf("Erreur suppression coursiers.\n");
   } else {
     printf("Coursiers supprimés.\n");
   }



   for (i = 0; i< N_RESTOS; i++) {     /* libération de la mémoire allouée au tableau des restos */
      free(restos[i]);
   }
   free(restos);

   exit(0);
}
