#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"
#include "semaphore.h"
#include "shmem.h"

void affiche_stock(PLAT* stock, int pid_resto){

   printf("Menu du resto %d\n", pid_resto);
   int i;
   for (i = 0; i < N_plats; i++) {
      printf("%d\t%s\t\t\tQuantité : %d\n", stock[i].id_plat, stock[i].name, stock[i].quantity );
   }

}

int main(){


   struct msgbuf msg_buf;

   PLAT* stock;

   key_t clef_message = ftok("hubert.c", 0);
   if (clef_message == -1) {
      printf("Erreur de création clé de la chaîne de messages.\n" );
      exit(-1);
   } //else printf("Clef chaîne de messages générée avec succès. = %d\n", clef_message);

   int msg_id = msgget(clef_message, 0666);
   if (msg_id == -1) {
      printf("Erreur de connexion à la chaîne de messages. Avez-vous lancé Hubert ?\n" );
      exit(-1);
   } //else printf("Connection à la CdM réussie. id = %d\n", msg_id);

   key_t clef_coursiers = ftok("hubert.c", 1);
   if (clef_coursiers == -1) {
      printf("Erreur de création clé des coursiers.\n" );
      exit(-1);
   } //else printf("Clef coursiers générée avec succès. = %d\n", clef_coursiers);

   int coursiers_id = open_semaphore(clef_coursiers);
   if (coursiers_id == -1) {
      printf("Erreur d'ouverture du sémaphore coursiers.\n" );
      exit(-1);
   } //else printf("Semaphore coursiers ouvert. id = %d\n", coursiers_id);


   printf("Bienvenue sur hungry hUBERt ! Que souhaitez vous manger aujourd'hui ?\n\t1 : Italien\n\t2 : Cuisine lyonnaise\n\t3 : Kebab\n");

   int type;

   int res = -1;

   while (res == -1) {
      scanf("%d", &type);

      msg_buf.id_resto = type;
      msg_buf.signature = getpid();
      msg_buf.mtype = 1; //destinataire
      msg_buf.type = 1;
      msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0);


      msgrcv(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), getpid(), 0); //attente de la réponse

      res = msg_buf.id_resto;
      if (res == -1) printf("Désolé, ce type de restaurant est indisponible pour le moment, veuillez en choisir un autre\n");
   }

   int id_resto = msg_buf.id_resto;

   int shm_id = open_shmem(id_resto, N_plats*sizeof(PLAT));
   if (shm_id == -1) {
      printf("Erreur ouverture mémoire du restaurant %d\n",msg_buf.id_resto);
      exit(-1);
   } //else printf("ShM ouverte. id = %d\n", shm_id);

   key_t clef_mutex = ftok("resto.c",id_resto);
   if (clef_mutex == -1) {
      printf("Erreur de création clé du mutex.\n" );
      exit(-1);
   } //else printf("Clef mutex générée avec succès. = %d\n", clef_mutex);

   int mutex_id = open_semaphore(clef_mutex);
   if (mutex_id  == -1) {
      printf("Erreur ouverture mutex du resto %d\n", id_resto);
      exit(-1);
   } //else printf("Mutex ouvert. id = %d\n", mutex_id);

   stock = attach_shmem(shm_id);
   printf("Adresse du stock : %p\n", stock);

   down(mutex_id);
   affiche_stock(stock, id_resto );
   up(mutex_id);

   detach_shmem(stock);

   res = -1;

   int id_plat;
   int quantity;

   while (res == -1) {
      printf("Veuillez taper l'ID correspondant au plat voulu.\n"); //à completer
      scanf("%d", &id_plat);
      printf("Combien en désirez vous ?\n"); //à completer
      scanf("%d", &quantity);
      msg_buf.signature = getpid();
      msg_buf.id_resto = id_resto;
      msg_buf.mtype = 1; //destinataire
      msg_buf.type = 2;
      msg_buf.id_plat = id_plat;
      msg_buf.quantity = quantity;
      msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0);

      msgrcv(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), getpid(), 0); //attente de la réponse

      res = msg_buf.quantity;
      if (res == -1) {
         printf("Erreur, veuillez sélectionner un ID et une quantité valides.\n" );
      }
   }
   sleep (10);
   up(coursiers_id);
   printf("Votre commande est arrivée ! Bon appétit et à bientôt.\n");
   exit(0);
}
