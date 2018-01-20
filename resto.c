#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "msg.h"
#include "semaphore.h"
#include "shmem.h"
#include "resto.h"

/*
*  Déclaration des variables.
*/
int mutex_id, shm_id, msg_id, pid_cuisine;
MSG msg_buf;
PLAT* stock;

void fermeture_resto() {

   kill(pid_cuisine,9);

   msg_buf.mtype = 1;
   msg_buf.signature = getpid();
   msg_buf.type = -1;
   msgsnd(msg_id, &msg_buf, sizeof(MSG) - sizeof(long), 0);

   /*
      Suppression des IPC.
   */
   int res = remove_shmem(shm_id);
   if (res == -1) {
     printf("Erreur suppression mémoire.\n");
   } else {
     printf("Mémoire supprimée.\n");
   }

   res = remove_semaphore(mutex_id);
   if (res == -1) {
     printf("Erreur suppression mutex.\n");
   } else {
     printf("Mutex supprimé.\n");
   }

   exit(0);
}

void sigusr1_handle(){}

int main() {

   /*
      Création des IPC
   */

   //creation mémoire partagée
   shm_id = create_shmem(getpid(), N_plats*sizeof(PLAT));
   if (shm_id == -1) {
     printf("Erreur création mémoire\n");
     exit(-1);
  } //else printf("ShM créée. id = %d\n", shm_id);

   //creation du mutex
   key_t clef_mutex = ftok("resto.c",getpid());
   if (clef_mutex == -1) {
      printf("Erreur de création clé du mutex.\n" );
      exit(-1);
   } //else printf("Clef mutex générée avec succès. = %d\n", clef_mutex);

   mutex_id = create_semaphore(clef_mutex);
   if (mutex_id  == -1) {
      printf("Erreur création mutex\n");
      exit(-1);
   } //else printf("Mutex créé. id = %d\n", mutex_id);

   init_semaphore(mutex_id,1);

   stock = attach_shmem(shm_id);


   pid_cuisine = fork();

   if (pid_cuisine > 0) { // CODE RESTO

      signal(SIGINT, fermeture_resto); //redirection du signal d'interruption

      //ouverture file de messages
      key_t clef = ftok("hubert.c", 0);
      if (clef == -1) {
         printf("Erreur de création clé de la chaîne de messages.\n" );
         exit(-1);
      } //else printf("Clef générée avec succès. = %d\n", clef);

      msg_id = msgget(clef, 0666);
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

      /*
         Initialistation resto
      */
      int type_resto;
      int res;
      do {
         printf("Entrer le type de votre restaurant\n\t-1 pour Italien\n\t-2 pour cuisine lyonnaise\n\t-3 pour Kebab\n");
         scanf("%d", &type_resto);
         down(mutex_id);
         res = enregistrer_resto(type_resto);
         up(mutex_id);
      } while (res == -1);

      kill(pid_cuisine, SIGUSR1); //réveil de la cuisine


      /*
         Comportement
      */
      while (1) {
         //attente d'une commande
         msgrcv(msg_id, &msg_buf, sizeof(MSG) - sizeof(long), getpid(), 0);

         printf("Reception d'une commande\n");
         //diminuer stock
         down(mutex_id);
         if (stock[msg_buf.id_plat].quantity >= msg_buf.quantity) {
            stock[msg_buf.id_plat].quantity -= msg_buf.quantity; //a changer
            //renvoyer réponse
            down(coursiers_id);
            msg_buf.mtype = 1;
            msg_buf.id_resto = msg_buf.signature; //ici correspond plutot à id_user
            msg_buf.signature = getpid();
            msg_buf.type = 3;
            msg_buf.quantity = 0;
            msgsnd(msg_id, &msg_buf, sizeof(MSG) - sizeof(long), 0);
         } else {
            printf("Stock insuffisant\n");
            msg_buf.mtype = 1;
            msg_buf.id_resto = msg_buf.signature; //ici correspond plutot à id_user
            msg_buf.signature = getpid();
            msg_buf.type = 3;
            msg_buf.quantity = -1;
            msgsnd(msg_id, &msg_buf, sizeof(MSG) - sizeof(long), 0);
         }

         afficher_stock();

         up(mutex_id);


      }
   } else if (pid_cuisine == 0) { //code de la cuisine

      signal(SIGUSR1,sigusr1_handle);
      pause(); //on attend que le resto se soit initialisé

      while (1) {
         sleep (10);
         down(mutex_id);
         int i;
         for (i=0; i<N_plats; i++) {
            if (stock[i].quantity < 10){
               stock[i].quantity++;
            }
         }
         afficher_stock();
         up(mutex_id);
      }
   }
}

void afficher_stock(){
   system("clear");
   printf("Stock actuel :\n");
   int i;
   for (i = 0; i < N_plats; i++) {
      printf("%d\t%s\t\t\tQuantité : %d\n", stock[i].id_plat, stock[i].name, stock[i].quantity );
   }

}

int enregistrer_resto(int type_resto){

   stock[0].id_plat = 0 ;
   stock[0].quantity = 5;
   stock[1].id_plat =  1;
   stock[1].quantity = 5;
   stock[2].id_plat =  2;
   stock[2].quantity = 5;
   stock[3].id_plat =  3;
   stock[3].quantity = 5;
   stock[4].id_plat =  4;
   stock[4].quantity = 5;

   switch (type_resto) {

      case 1 :
         strcpy(stock[0].name,"Penne sicilienne        ");
         strcpy(stock[1].name,"Spaghetti Puttanesca    ");
         strcpy(stock[2].name,"Bruschetta              ");
         strcpy(stock[3].name,"Carpaccio de boeuf      ");
         strcpy(stock[4].name,"Pizza napolitaine       ");
         break;

      case 2 :
         strcpy(stock[0].name,"Andouillette aux cardons");
         strcpy(stock[1].name,"Boudin lyonnais         ");
         strcpy(stock[2].name,"Tablier de sapeur       ");
         strcpy(stock[3].name,"Joue de boeuf           ");
         strcpy(stock[4].name,"Rognons d'agneau        ");
         break;

      case 3 :
         strcpy(stock[0].name,"Maxi kebab              ");
         strcpy(stock[1].name,"Menu kebab frites       ");
         strcpy(stock[2].name,"Tacos                   ");
         strcpy(stock[3].name,"Royal tacos             ");
         strcpy(stock[4].name,"Frites                  ");
         break;

      default :
         return -1;
   }


   msg_buf.mtype = 1;
   msg_buf.signature = getpid();
   msg_buf.type = 0;
   msg_buf.id_resto = type_resto;
   msgsnd(msg_id, &msg_buf, sizeof(MSG) - sizeof(long), 0);

   return 0;

}
