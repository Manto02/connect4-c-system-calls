#include "shared_memory.h"
#include "errExit.h"

//effettua la detach dal segmento di shared memory
void free_shared_memory (void *ptr_sh) {
    if (shmdt(ptr_sh) == -1) {
        errExit("smhdt failed");
    }
}

//rimuove il segmento di shared memory eliminando il suo id
void remove_shared_memory (int shmid) {
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        errExit("shmctl failed");
    }
}

//crea un segmento di shared memory
int get_shared_memory (key_t shmKey, size_t size) {
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1) {
        errExit("shmget failed");
    }
    return shmid;
}

//effettua l'attach al segmento di shared memory
void *attach_shared_memory (int shmid, int shmflg) {
    void *ptr_sh = shmat(shmid, NULL, shmflg);
    if (ptr_sh == (void *)-1) {
        errExit("smhat failed");
    }
}

/************************************
*Matricola VR471881, VR474454, 473647
*Nome e Cognome Simone Rossato, Matteo Mantovani, Thomas Lonardi
*Data di realizzazione 4 luglio 2023
*************************************/
