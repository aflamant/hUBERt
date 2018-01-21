#ifndef _HUBERT_TYPES_H
#define _HUBERT_TYPES_H

#define N_plats 5

typedef struct msgbuf{
        long mtype; //pid de destinataire
        int signature; //pid d'expéditeur
        int type; //type de message
        int id_resto; //type de resto ou pid de resto
        int quantity; //quantité commandé
        int id_plat; //type de plat commandé
} MSG;

typedef struct plat {
    int id_plat;
    int quantity;
    char name[50];
} PLAT;

#endif
