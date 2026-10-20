#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "semaphore.h"
#include "errExit.h"

//dati semid, l' n-esimo semaforo del semaphore set creato su cui vogliamo effettuare la semop e l'operazione che vogliamo fare, effettua la semop su quel determinato semaforo del semaphore set
void semOp(int semid, unsigned short sem_num, short sem_op){
    static struct sembuf sops = {0};

    sops.sem_num = sem_num;
    sops.sem_op = sem_op;

    int ret;
    do {
        ret = semop(semid, &sops, 1);
    } while (ret < 0 && errno == EINTR);
    if (ret < 0 && errno != -1) {
        errExit("semop failed");
    }
}

/************************************
*Matricola VR471881, VR474454, 473647
*Nome e Cognome Simone Rossato, Matteo Mantovani, Thomas Lonardi
*Data di realizzazione 4 luglio 2023
*************************************/