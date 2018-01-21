#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "hubert_types.h"
#include "user.h"

#include "semaphore.h"
#include "shmem.h"

int main(){
   /* ***************************************
   *    Connexion à la chaîne de messages
   * ***************************************/
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
   MSG msg_buf;


   /* **************************************
   *    Connexion au sémaphore coursiers
   * **************************************/
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



   /* *********************************
   *     Début du programme user
   * *********************************/
   printf("Bienvenue sur hungry hUBERt ! Que souhaitez vous manger aujourd'hui ?\n\t1 : Italien\n\t2 : Cuisine lyonnaise\n\t3 : Kebab\n");

   int type;   /* déclaration de l'entier de réception du scanf */
   int res = -1;

   while (res == -1) {

      scanf("%d", &type);

      msg_buf.id_resto = type;
      msg_buf.signature = getpid();
      msg_buf.mtype = 1; //destinataire
      msg_buf.type = 1;
      msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0); /* Envoi à hubert de la demande d'offre du type selectionné */


      msgrcv(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), getpid(), 0); //attente de la réponse

      res = msg_buf.id_resto;    /* Si il n'existe pas de restaurant de ce type, res = -1 */
      if (res == -1) printf("Désolé, ce type de restaurant est indisponible pour le moment, veuillez en choisir un autre\n");
   }
   int id_resto = res;



   /* ****************************************************
   *        Ouverture de la mémoire du resto
   * ****************************************************/
   int shm_id = open_shmem(id_resto, N_plats*sizeof(PLAT));
   if (shm_id == -1) {
      printf("Erreur ouverture mémoire du restaurant %d\n",msg_buf.id_resto);
      exit(-1);
   } //else printf("ShM ouverte. id = %d\n", shm_id);


   /* ***************************************************
   *           Ouverture du mutex du resto
   * ***************************************************/
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




   PLAT* stock = attach_shmem(shm_id); /* on s'attache à la mémoire du resto */

   down(mutex_id);
   afficher_menu(stock, id_resto );    /* on affiche le menu du resto (en excluant les autres processus de la zone critique pendant la consutation) */
   up(mutex_id);

   detach_shmem(stock);                /* on se détache de la mémoire du resto */


   int id_plat;   /* déclaration des entiers de reception du scanf */
   int quantity;

   res = -1;

   while (res == -1) {
      printf("Veuillez taper l'ID correspondant au plat voulu.\n");
      scanf("%d", &id_plat);
      printf("Combien en désirez vous ?\n");
      scanf("%d", &quantity);
      msg_buf.signature = getpid();
      msg_buf.id_resto = id_resto;
      msg_buf.mtype = 1; //destinataire
      msg_buf.type = 2;
      msg_buf.id_plat = id_plat;
      msg_buf.quantity = quantity;
      msgsnd(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), 0);   /* envoi de la commande au resto via hubert */

      msgrcv(msg_id, &msg_buf, sizeof(MSG)-sizeof(long), getpid(), 0); /* attente de la réponse */

      res = msg_buf.quantity; /* si la commande était invalide, res = -1 */
      if (res == -1) {
         printf("Erreur, veuillez sélectionner un ID et une quantité valides.\n" );
      }
   }

   printf("Votre commande est en chemin !\n"); /* si la commande est valide, on sort de la boucle */
   sleep (10);    /* on simule le temps d'arrivée du coursier par un sleep */
   up(coursiers_id);    /* quand la commande est arrivée, on peut libérer le coursier */
   printf("Votre commande est arrivée ! Bon appétit et à bientôt.\n");
   exit(0);
}


void afficher_menu(PLAT* stock, int pid_resto){

   printf("Menu du resto %d\n", pid_resto);
   int i;
   for (i = 0; i < N_plats; i++) {
      printf("%d\t%s\t\t\tQuantité : %d\n", stock[i].id_plat, stock[i].name, stock[i].quantity ); /* On parcoure le tableau en affichant les produits disponibles */
   }

}
