#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "msg.h"
#include "semaphore.h"
#include "shmem.h"

#define N_COURSIERS 5


int coursiers_id, msg_id;
int* index_restos;
int** restos;

void ajouter_resto(int pid, int type){
   int i = 0;
   while (i<N_RESTOS && restos[i][0] != 0) {
      i++;
   }
   if (i < N_RESTOS ) {
      restos[i][0] = pid;
      restos[i][1] = type;
   } else kill(pid,SIGINT);
}

void supprimer_resto(int pid) {
   int i =0;
   while(i<N_RESTOS && restos[i][0]!=pid) {
      i++;
   }
   if (i<N_RESTOS) {
      restos[i][0] = 0;
      restos[i][1] = 0;
      printf("Resto %d supprimé.\n",pid );
   }
}

int chercher_resto(int **tab, int id_resto){
   int i=0;
   while (i < N_RESTOS){
     if (tab[i][1] == id_resto){
         return tab[i][0];
     }
     i++;
   }

   return -1;
}

void fermeture_hubert() {
   int i;
   for (i = 0; i<N_RESTOS; i++) {
      if (restos[i][0]!=0)
         kill(restos[i][0],SIGINT);
   }

   int res = msgctl (msg_id, IPC_RMID, NULL);
   if (res == -1) {
     printf("Erreur suppression file de messages.\n");
   } else {
     printf("File de messages supprimée.\n");
   }

   res = remove_semaphore(coursiers_id);
   if (res == -1) {
     printf("Erreur suppression coursiers.\n");
   } else {
     printf("Coursiers supprimés.\n");
   }



   for (i = 0; i< N_RESTOS; i++) {
      free(restos[i]);
   }
   free(restos);
   free(index_restos);
   exit(0);
}

int main(){

   signal(SIGINT, fermeture_hubert);

   MSG msg_buf;

   index_restos = (int*)malloc(sizeof(int));
   *index_restos = 0;

   restos = (int**)malloc(N_RESTOS*sizeof(int*)); //pointeur vers tableau 2d de restos avec les pid et les types
   int i;
   for (i = 0; i< N_RESTOS; i++) {
      restos[i] = (int*)malloc(2*sizeof(int));
   }

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
   printf("Initialistation de hUBERt.\n");
   printf("Nombre de coursiers présents aujourd'hui : %d\n", N_COURSIERS);

   while(1){

      msg_buf.type = 0;

      msgrcv(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 1, 0); // réception de message destiné à hubert

      int pid_resto;
      int i;

      switch (msg_buf.type) {

         case -1 : //fermeture de hubert
            supprimer_resto(msg_buf.signature);
            break;

        case 0 : // pour l'inscription de resto
            ajouter_resto(msg_buf.signature, msg_buf.id_resto);
            printf("Resto de type %d ajouté : %d\n", msg_buf.id_resto, msg_buf.signature);
            break;

        case 1: // pour le demande de l'offre
            printf("Demande d'offre de restaurant de type %d la part de %d.\n", msg_buf.id_resto, msg_buf.signature );
            pid_resto = chercher_resto(restos, msg_buf.id_resto);
            printf("Envoi de la carte de %d\n", pid_resto );
            msg_buf.mtype = msg_buf.signature;
            msg_buf.signature = 0;
            msg_buf.type = 0;
            msg_buf.id_resto = pid_resto;
            msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0);
            break;

        case 2: // pour une commande
            printf("Commande de la part de l'utilisateur %d vers le restaurant %d\n", msg_buf.signature, msg_buf.id_resto );
            msg_buf.mtype = msg_buf.id_resto;
            msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0);
            break;

        case 3: // pour une réponse d'une commande
            printf("La commande de %d pour %d vient d'être envoyée.\n", msg_buf.signature, msg_buf.id_resto );
            msg_buf.mtype = msg_buf.id_resto; // id_resto est utilisé comme id_user
            msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0);
            up(coursiers_id);
            break;
      }
   }
}
