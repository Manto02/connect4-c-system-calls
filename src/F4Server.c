#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define PATH_MAX 500
#define MOVE_TIME 10


#include "errExit.h"
#include "msg_queue.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "controllo.h"
#include "matrix.h"
#include "segnale.h"


#define SERVER 0
#define CLIENT 1
#define PLAYER1 2
#define PLAYER2 3

//./F4Server 6 7 O X

int main (int argc, char *argv[]) {
    //gestione caso di errore nell' inizializzazione del programma
    if (argc != 5) {
        printf("<SERVER> Errore di inizializzazione: bisogna compilare ./F4Server maxriga maxcolonna gettone1 gettone2\n");
        exit(1);
    }
    //dichiarazione e inizializzazione variabili
    char path[PATH_MAX];
    char gettone1 = argv[3][0];
    char gettone2 = argv[4][0];
    char giocatore1[80];
    char giocatore2[80];
    char giocatore_casuale[] = "CPU";

    //gestione dei segnali creando un proprio set di segnali andando ad utilizzare una mask per selezionare quali segnali bloccare e non
    sigset_t mySet;
    sigfillset(&mySet);
    sigdelset(&mySet, SIGINT);//CTRL+C doppio
    sigdelset(&mySet, SIGALRM);//per chiusura totale
    sigdelset(&mySet,SIGUSR1);//abbandono client
    sigdelset(&mySet,SIGUSR2);//scadenza tempo
    sigdelset(&mySet, SIGTERM);//se accade un errExit
    sigprocmask(SIG_SETMASK, &mySet, NULL);
    
    //sostituzione dell'handler di default dei vari segnali con quelli creati
    if (signal(SIGINT, fineserver_handler1) == SIG_ERR)
        errExit("signal failed");
    if (signal(SIGUSR1, abbandono_handler) == SIG_ERR)
        errExit("signal failed");
    if (signal(SIGUSR2, tempo_scaduto_handler) == SIG_ERR)
        errExit("signal failed");
    if (signal(SIGTERM, errExit_server) == SIG_ERR)
        errExit("signal failed");
    srand(time(NULL));
    fase = 0;
    
    int riga = atoi(argv[1]);
    int colonna = atoi(argv[2]);

    //controlli dimensione griglia di gioco
    if (riga < 5 || colonna < 5) {
        printf("<SERVER> Valori per la griglia errati. La griglia minima deve essere una 5x5\n");
        exit(1);
    } else 
        printf("<SERVER> Controllo valori colonna e riga per la griglia andato a buon fine...\n");

    //controllo validità gettoni
    if (strcmp(argv[3], argv[4]) == 0) {
        printf("<SERVER> I gettoni per i giocatori sono uguali. Devono essere diversi\n");
        exit(1);
    } else 
        printf("<SERVER> Controllo gettoni andato a buon fine...\n");
    

    //recupero CWD e ftok per key shared memory e semafori
    if(getcwd(path, PATH_MAX) == NULL) {
        printf("<CLIENT> getcwd failed\n");
        exit(1);
    }
    
    //generazione chiavi per IPC con ftok
    //-------------------------------------------------------------------------------------------------
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
    //-------------------------------------------------------------------------------------------------
    
    

    //creazione semaforo
    semid = semget(smkey, 4, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (semid == -1)
        errExit("<SERVER> semget failed");
    
    //assegnamento valori ai semafori
    unsigned short int values[] = {0, 0, 0, 0};
    arg.array = values;
    if (semctl(semid, 0, SETALL, arg) == -1)
        errExit("<SERVER> semctl failed");

    //creazione message queue
    msqid = msgget(msgkey, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("<SERVER> msgget failed");

    //creazione tabellone di gioco e allocazione in shared memory
    printf("\n<SERVER> allocating a shared_memory segment...\n");
    shmid = get_shared_memory(shmkey, sizeof(char) * riga * colonna);
    char *tabella = (char*) attach_shared_memory(shmid, 0);
    
    for(int i = 0; i < riga * colonna; i++){
        tabella[i] = ' ';
    }

   //recupero pid server e invio al client
    pidServer = getpid();
    carSize = sizeof(struct mymsgc) - sizeof(long);
    intSize = sizeof(struct mymsgi) - sizeof(long);
    printf("<SERVER> Inviando pid al giocatore 1\n");
    struct mymsgpid pId = {PID_SERVER, pidServer};
    if (msgsnd(msqid, &pId, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    semOp(semid, CLIENT, 1); //1

    //attesa per ricevere il pid del giocatore 1
    semOp(semid, SERVER, -1); //2
    if (msgrcv(msqid, &pId, carSize, PID_CLIENT, 0) == -1) {
        errExit("<SERVER> msgrcv failed");
    }
    pidGiocatore1 = pId.mint;
    fase = 1;

    //invio dimensione tabella, gettoni e tempo al client1
    printf("<SERVER> Inviando numero massimo colonna al giocatore 1\n");
    struct mymsgi column = {COLONNA, colonna};
    if (msgsnd(msqid, &column, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    
    printf("<SERVER> Inviando numero massimo riga al giocatore 1\n");
    struct mymsgi row = {RIGA, riga};
    if (msgsnd(msqid, &row, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");

    printf("<SERVER> Inviando gettone %c al giocatore 1\n", gettone1);
    struct mymsgc gettone = {GETTONE, gettone1};
    if (msgsnd(msqid, &gettone, carSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");

    printf("<SERVER> Inviando il tempo massimo d'attesa al giocatore 1\n");
    struct mymsgi time = {TIME, MOVE_TIME};
    if (msgsnd(msqid, &time, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    semOp(semid, CLIENT, 1); //3
    semOp(semid, SERVER, -1); //4
    
    //invio shared_memory id
    struct mymsgi _shared = {SHM_ID, shmid};
    if (msgsnd(msqid, &_shared, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    semOp(semid, CLIENT, 1); //5
    semOp(semid, SERVER, -1);//6

    //attesa messaggio nome giocatore 1 dal client
    struct mymsgc player;
    if (msgrcv(msqid, &player, carSize, NOME_GIOCATORE, 0) == -1) {
        errExit("<SERVER> msgrcv failed");
    }
    strcpy(giocatore1, player.mtext);

    //invio al client il valore 1 per indicare che è presente il giocatore1
    struct mymsgi player1 = {PRESENZA_GIOCATORE_1, 1};
    if (msgsnd(msqid, &player1, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    semOp(semid, CLIENT, 1); //7
    semOp(semid, SERVER, -1); //8

    //recupero del carattere speciale per controllare se è stata decisa una partita contro la CPU
    struct mymsgc choice;
    if(msgrcv(msqid, &choice, carSize, SCELTA_ASTERISCO, IPC_NOWAIT) != -1)
        scelta = choice.mtext[0];
    
    if (scelta == '*') {
        char coin_CPU = gettone2;
        printf("\n<SERVER> Inizio preparazione partita contro CPU\n");
        pid_t pid = fork();
        if (pid == -1) {
            errExit("Fork failed\n");
        } else if (pid == 0) {
            if(execlp("bin/F4Client", "F4Client", "CPU", (char *)NULL) == -1)
                errExit("execl failed");
        }
    } 
    //invio e prelevazione informazioni client 2
    //invio pid server a client 2
    printf("\n<SERVER> Inviando pid al giocatore 2\n");
    struct mymsgi p = {PID_SERVER, pidServer};
    if (msgsnd(msqid, &p, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    semOp(semid, CLIENT, 1); //1
    semOp(semid, SERVER, -1); // 2
    //attesa di ricevere il pid del client 2 
    if (msgrcv(msqid, &p, carSize, PID_CLIENT, 0) == -1) {
        errExit("<SERVER> msgrcv failed");
    }
    pidGiocatore2 = p.mint;
    fase = 2;

    //invio informazioni gioco al client 2, in ordine: colonna, riga, gettone, tempo, shared memory id
    printf("<SERVER> Inviando numero massimo colonna al giocatore 2\n");
    if (msgsnd(msqid, &column, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    printf("<SERVER> Inviando numero massimo riga al giocatore 2\n");
    if (msgsnd(msqid, &row, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    printf("<SERVER> Inviando gettone %c al giocatore 2\n", gettone2);
    struct mymsgc getton = {GETTONE, gettone2};
    if (msgsnd(msqid, &getton, carSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");

    printf("<SERVER> Inviando il tempo massimo d'attesa al giocatore 2\n");
    if (msgsnd(msqid, &time, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    semOp(semid, CLIENT, 1); //3
    semOp(semid, SERVER, -1); //4

    if (msgsnd(msqid, &_shared, intSize, 0) == -1)
        errExit("<SERVER> msgsnd failed");
    semOp(semid, CLIENT, 1); //5
    semOp(semid, SERVER, -1); //6

    //attesa del nome del giocatore 2
    if (msgrcv(msqid, &player, carSize, NOME_GIOCATORE, 0) == -1) {
        errExit("<SERVER> msgrcv failed");
    }
    strcpy (giocatore2, player.mtext);

    semOp(semid, CLIENT, 1); //7
    
    //visualizzazione chiara dei due giocatori con i loro gettoni 
    printf("\n\n<GAMEMASTER> Giocatore 1: %s Gettone: %c\n", giocatore1, gettone1);
    printf("<GAMEMASTER> Giocatore 2: %s Gettone: %c\n", (scelta == '*') ? giocatore_casuale : giocatore2, gettone2);
    printf("<GAMEMASTER> La partita è iniziata\n");
    printf("<GAMEMASTER> E' il turno di %s\n", giocatore1);
    
    semOp(semid, SERVER,-1); //8
    semOp(semid, PLAYER1,1); //9

    //fine[0] indica se uno dei due giocatori ha inserito la mossa vincente, fine[1] indica una situazione di parità
    int fine[2] = {0, 1};
    struct mymsgi win = {15, 4};
    struct mymsgi lose = {15, 5};
    //creazione matrice ausiliaria per semplificare i controlli sulla shared memory
    char **matrice;
    matrice = (char **)malloc(riga * sizeof(char*));
    for (int x = 0; x < riga; x++)
        matrice[x] = (char *)calloc(colonna, sizeof(char));    

    semOp(semid, SERVER, -1);
    //controllo vero e proprio del gioco
    do {
        //ricevuto il gettone si rende il segmento di shared memory (array) in una matrice, si effettuano i controlli su di essa. Infine si riporta la matrice in array
        if (msgrcv(msqid, &gettone, carSize, GETTONE, 0) == -1)
            errExit("<SERVER> msgrcv failed");
        char coin = gettone.mtext[0];
        array_to_matrix(tabella, matrice, riga, colonna);

        controllo_orizzontale(matrice, riga, colonna, fine, coin);
        if (fine[0] == 0)
            controllo_verticale(matrice, riga, colonna, fine, coin);
        if (fine[0] == 0)
            controllo_diagonale(matrice, riga, colonna, fine, coin);
        if (fine[0] == 0)
            controllo_pari(matrice, riga, colonna, fine);
        
        matrix_to_array(tabella, matrice, riga, colonna);
        
        //se dai controlli risulta una vittoria
        if (fine[0] == 1) {
            //if per controllare se ha vinto il giocatore 1 o il giocatore 2
            if(coin == gettone1) {
                msgsnd(msqid, &win, intSize, 0);
                kill(pidGiocatore1, SIGUSR1);
                if (scelta != '*') {
                    semOp(semid, SERVER, -1);
                }
                msgsnd(msqid, &lose, intSize, 0);
                kill(pidGiocatore2, SIGUSR1);
            } else {
                if (scelta != '*')
                    msgsnd(msqid, &win, intSize, 0);
                
                kill(pidGiocatore2, SIGUSR1);
                if (scelta != '*')
                    semOp(semid, SERVER, -1);
                
                msgsnd(msqid, &lose, intSize, 0);
                kill(pidGiocatore1, SIGUSR1);
            }
        //se non c'è stata vittoria si controlla se c'è stata parità
        } else if (fine[1] == 1) {
            kill(pidGiocatore2, SIGUSR2);
            kill(pidGiocatore1, SIGUSR2);
        //altrimenti il server (arbitro) fa il cambio turno
        } else {
            if(coin == gettone1) {
                printf("<GAMEMASTER> E' il turno di %s\n", giocatore2);
                semOp(semid,PLAYER2,1);
                semOp(semid,SERVER,-1);
            } 
            else {
                printf("<GAMEMASTER> E' il turno di %s\n", giocatore1);
                semOp(semid,PLAYER1,1);
                semOp(semid,SERVER,-1);
            }
        }
    } while (!fine[0] && !fine[1]);

    if (signal(SIGUSR1, chiusura_totale_handler) == SIG_ERR)
        errExit("signal failed");
    pause();

    return 0;
}

/************************************
*Matricola VR471881, VR474454, 473647 
*Nome e Cognome Simone Rossato, Matteo Mantovani, Thomas Lonardi
*Data di realizzazione 5 settembre 2023
*************************************/
    





