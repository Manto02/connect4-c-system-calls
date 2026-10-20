#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

#include "shared_memory.h"
#include "semaphore.h"
#include "msg_queue.h"
#include "errExit.h"
#include "matrix.h"
#include "controllo.h"
#include "segnale.h"

#define PATH_MAX 500

#define SERVER 0
#define CLIENT 1
#define PLAYER1 2
#define PLAYER2 3

//./F4Client NomeUtente \* se deve giocare la CPU

int main (int argc, char *argv[]) {
    //controllo parametri di avvio    
    if (argc < 2) {
        printf("<CLIENT> Errore di inizializzazione." 
                "\nBisogna compilare ./F4Client NomeUtente * (quest'ultimo deve essere presente e preceduto da un backslash se si vuole iniziare una partita contro la CPU)\n");
        exit(1);
    }
    
    //dichiarazione e inizializzazione variabili
    char path[PATH_MAX];
    momento = 0;

    srand(time(NULL));

    //creazione del set di segnali
    sigset_t mySet;
    sigfillset(&mySet);
    if (strcmp(argv[1], "CPU") != 0) {
        sigdelset(&mySet, SIGINT);//abbandono
        sigdelset(&mySet, SIGALRM);//tempo attesa
        sigdelset(&mySet,SIGUSR1);//casi_client
        sigdelset(&mySet,SIGUSR2);//parità
        sigdelset(&mySet, SIGTERM);//se accade un errExit
        sigprocmask(SIG_SETMASK, &mySet, NULL);
        //cambio del sighandler di default con quello creato
        if (signal(SIGINT, finegiocatore_handler) == SIG_ERR) {
            printf("signal failed\n");
            exit(1);
        }
            
        if (signal(SIGALRM, errExit_client) == SIG_ERR) {
            printf("signal failed\n");
            exit(1);
        }
        if (signal(SIGUSR1, casi_client_handler) == SIG_ERR) {
            printf("signa failed\n");
            exit(1);
        }
        if (signal(SIGUSR2, pari_client) == SIG_ERR) {
            printf("signal failed\n");
            exit(1);
        }
        if (signal(SIGTERM, errExit_client) == SIG_ERR) {
            printf("signal failed\n");
            exit(1);
        }
    }
    
    //recupero chiavi IPC
    if(getcwd(path, PATH_MAX) == NULL) {
        printf("<CLIENT> getcwd failed\n");
        exit(1);
    }
    
    key_t shmkey = ftok(path, 2);
    if (shmkey == -1) {
        printf("<CLIENT> ftok failed (shmkey not create)\n");
        exit(1);
    }

    key_t smkey = ftok(path, 3);
    if (smkey == -1) {
        printf("<CLIENT> ftok failed (smkey not create)\n");
        exit(1);
    }

    key_t msgkey = ftok(path, 1);
    if (msgkey == -1) {
        printf("<CLIENT> ftok failed (msgkey not create)\n");
        exit(1);
    }
    //recupero IPC
    semid = semget(smkey, 4, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1) {
        printf("<CLIENT> semget failed\n");
        exit(1);
    }
    msqid = msgget(msgkey, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqid == -1) {
        printf("<CLIENT> msgget failed\n");
        exit(1);
    }

    //creazione struct per messaggi
    carSize = sizeof(struct mymsgc) - sizeof(long);
    intSize = sizeof(struct mymsgi) - sizeof(long);
    struct mymsgi colonna;
    struct mymsgi riga;
    struct mymsgi _shared;
    struct mymsgi time;
    struct mymsgpid pId;
    
    //recupero pid server
    //se per caso c'è già una partita in corso o non c'è un server, attende 3 secondi e poi termina
    signal(SIGALRM, errExit_client);
    alarm(3);
    semOp(semid, CLIENT, -1); //1
    
    alarm(0);
    if (signal(SIGALRM, timer_handler) == SIG_ERR) {
        printf("signal failed\n");
        exit(1);
    }

    if (msgrcv(msqid, &pId, intSize, PID_SERVER, 0) == -1) {
        errExit("<CLIENT> msgrcv failed");
    }
    pidServer = pId.mint;
    
    //invio pid client al server
    pid_t pidClient = getpid();
    pId.mtype = PID_CLIENT;
    pId.mint = pidClient;
    if (msgsnd(msqid, &pId, intSize, 0) == -1)
        errExit("<CLIENT> msgsnd failed");
    semOp(semid, SERVER, 1); //2
    semOp(semid, CLIENT, -1); //3
    
    //attesa dimensioni tabella di gioco
   
    if (msgrcv(msqid, &colonna, intSize, COLONNA, 0) == -1) {
        errExit("<CLIENT> msgrcv failed");
    }
    int max_colonna = colonna.mint;

    
    if (msgrcv(msqid, &riga, intSize, RIGA, 0) == -1) {
        errExit("<CLIENT> msgrcv failed");
    }
    int max_riga = riga.mint;

    //recupero gettone
    struct mymsgc gettone;

    if (msgrcv(msqid, &gettone, carSize, GETTONE, 0) == -1) {
        errExit("<CLIENT> msgrcv failed");
    }

    if (strcmp(argv[1], "CPU") != 0) {
        printf("<CLIENT> Giocatore: %s\n", argv[1]);
        printf("<CLIENT> Gettone: %c\n", gettone.mtext[0]);
    }
    char coin = gettone.mtext[0];

    //attesa per ricevere tempo di time_out
    if (msgrcv(msqid, &time, intSize, TIME, 0) == -1) {
        errExit("<CLIENT> msgrcv failed");
    }
    int tempo = time.mint;
    semOp(semid, SERVER, 1); //4
    semOp(semid, CLIENT, -1);  //5

    //attesa per ricevere tabella di gioco
    if (msgrcv(msqid, &_shared, intSize, SHM_ID, 0) == -1){
        errExit("<CLIENT> msgrcv failed");
    }
    shmid = _shared.mint;
    char *tabella;
    //creazione matrice ausiliaria per l'inserimento del gettone
    char **matrice;
    matrice = (char **)malloc(max_riga * sizeof(char*));
    for (int x = 0; x < max_riga; x++)
        matrice[x] = (char *)calloc(max_colonna, sizeof(char));
    //attached al segmento di shared memory
    tabella = (char*) attach_shared_memory(shmid, 0);

    //invio nome giocatore al server
    char *nome = argv[1];
    struct mymsgc giocatore;
    giocatore.mtype = NOME_GIOCATORE;
    strcpy(giocatore.mtext, nome);

    if (msgsnd(msqid, &giocatore, carSize, 0) == -1)
        errExit("<CLIENT> msgsnd failed");
    semOp(semid, SERVER, 1); //6
    semOp(semid, CLIENT, -1); //7

    //controllo presenza giocatore 1 per capire se sono il client 2
    struct mymsgi player;
    int presenza = 0;
    if(msgrcv(msqid, &player, intSize, PRESENZA_GIOCATORE_1, IPC_NOWAIT) != -1)
    presenza = player.mint;

    char gettone1;
    //controlla se è già stato scelto il primo giocatore quindi vedi se attendere il secondo giocatore o se si è contro CPU
    
    if(presenza) {
        //controllo per vedere se si è contro la CPU
        if (argc >= 3 && argv[2][0] == '*') {
            scelta = argv[2][0];
            struct mymsgc casual = {SCELTA_ASTERISCO, scelta};
            printf("\n<CLIENT> Iniziando preparazione alla partita contro la CPU\n");
            if (msgsnd(msqid, &casual, carSize, 0) == -1)
                errExit("<CLIENT> msgsnd failed");
        } 
        //altrimenti si manda un valore diverso da * al server e si attende la ricerca del secondo giocatore
        else {
            struct mymsgc casual = {SCELTA_ASTERISCO, '/'};
            scelta = '/';
            if (msgsnd(msqid, &casual, carSize, 0) == -1)
                errExit("<CLIENT> msgsnd failed");
            printf("\n<CLIENT> Attendere ricerca del secondo giocatore...\n");
        }
        semOp(semid,SERVER,1); //8
        //inizializzazione effettuata solo dal client 1, in questo modo si possono gestire i turni utilizzando qualsiasi gettone
        gettone1 = coin;
        printf("\nAttendi il tuo turno...\n");
        semOp(semid,PLAYER1, -1); //9
    } 
    else {//sono client 2
        semOp(semid,SERVER,1); //8
        momento = 1;
        if (strcmp(argv[1], "CPU") != 0)
            printf("\nAttendi il tuo turno...\n");
        semOp(semid,PLAYER2, -1);
    }

    int fine = 0;

    momento = 1;
    if (strcmp(argv[1], "CPU") != 0) {
        //rende il segmento di shared memory (array) in matrice per l'inserimento del gettone e per la stampa. Infine riporta la matrice in array e aspetta il prossimo turno
        do {
            array_to_matrix(tabella, matrice, max_riga, max_colonna);
            stampa_matrix(matrice, max_riga, max_colonna);
            //grazie a gettone1 inizializzato precedentemente solo il giocatore 1 può entrare in questa condizione
            if (coin == gettone1) {
                struct mymsgc gettone = {GETTONE, coin};
                if (msgsnd(msqid, &gettone, carSize, 0) == -1)
                    errExit("<CLIENT> msgsnd failed");

                inserimento_manuale_gettone(matrice, max_riga, max_colonna, gettone.mtext, TIME);
                stampa_matrix(matrice, max_riga, max_colonna);
                matrix_to_array(tabella, matrice, max_riga, max_colonna);

                //attesa prossimo turno
                semOp(semid,SERVER,1);
                printf("Attendi il tuo turno...\n");
                semOp(semid,PLAYER1,-1);
                
            }
            else {
                struct mymsgc gettone = {GETTONE, coin};
                if (msgsnd(msqid, &gettone, carSize, 0) == -1)
                    errExit("<CLIENT> msgsnd failed");
                inserimento_manuale_gettone(matrice, max_riga, max_colonna, gettone.mtext, TIME);
                stampa_matrix(matrice, max_riga, max_colonna);
                matrix_to_array(tabella, matrice, max_riga, max_colonna);
                
                //attesa prossimo turno
                semOp(semid,SERVER,1);
                printf("Attendi il tuo turno...\n");
                semOp(semid,PLAYER2,-1);
            }
        } while(!fine);
    } else {
        //se si è la CPU si comporta come il secondo client, ma effettua l'inserimento automatico e casuale
        do {
            struct mymsgc gettone = {GETTONE, coin};
            if (msgsnd(msqid, &gettone, carSize, 0) == -1)
                errExit("<CLIENT> msgsnd failed");
            array_to_matrix(tabella, matrice, max_riga, max_colonna);
            inserimento_casuale_gettone(matrice, max_riga, max_colonna, coin);
            matrix_to_array(tabella, matrice, max_riga, max_colonna);
            
            //attesa prossimo turno
            semOp(semid,SERVER,1);
            semOp(semid,PLAYER2,-1);
        } while (!fine);
    }
    return 0;
}

/************************************
*Matricola VR471881, VR474454, 473647
*Nome e Cognome Simone Rossato, Matteo Mantovani, Thomas Lonardi
*Data di realizzazione 5 settembre 2023
*************************************/
