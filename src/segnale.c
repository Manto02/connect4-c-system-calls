#include "segnale.h"

pid_t pidServer;
pid_t pidGiocatore1;
pid_t pidGiocatore2;

int momento;
int fase;
char scelta;
int mich;

int shmid;
int msqid;
int semid;

size_t carSize;
size_t intSize;
union semun arg;

//segnali utilizzati dal client

//segnale per indicare al client che sul server è avvenuto il doppio CTRL+C
void chiusura_server_handler (int signum) {
    printf("\n<CLIENT> Ci scusiamo ma per problemi tecnici il server non è più raggiungibile\n");
    kill(pidServer, SIGUSR1);
    _exit(0);
}

//segnale per indicare la parità
void pari_client (int signum) {
    printf("\n<CLIENT> La partita e' terminata in parita'\n"
           "\n<CLIENT> Grazie per aver giocato\n");
        kill(pidServer, SIGUSR1);
    _exit(0);
}

//segnale per indicare la vittoria nel caso di abbandono del secondo giocatore
void vittoria_abbandono_handler (int signum) {
    printf("\n<CLIENT> L'altro giocatore ha abbandonato.\n"
           "<CLIENT> Congratulazioni, hai vinto\n"
           "<CLIENT> Grazie per aver giocato\n");
    _exit(0);
};

//segnale per indicare la vittoria nel caso che al secondo giocatore scada il tempo limite per effettuare la mossa
void vittoria_tempo_handler (int signum) {
    printf("\n<CLIENT> L'altro giocatore ha perso per inattività.\n"
           "<CLIENT> Congratulazioni, hai vinto\n"
           "<CLIENT> Grazie per aver giocato\n");
    _exit(0);
};

//segnale per indicare la vittoria della partita
void vittoria_handler (int signum) {
    printf("\n<CLIENT> Hai vinto!\n"
           "<CLIENT> Grazie per aver giocato\n");
    if (scelta == '*')
        kill(pidServer, SIGUSR1);
    else 
        semOp(semid, 0, 1);
    _exit(0);
}

//segnale per indicare la sconfitta della partita 
void sconfitta_handler (int signum) {
    printf("\n<CLIENT> Hai perso\n"
           "<CLIENT> Grazie per aver giocato\n");
    kill(pidServer, SIGUSR1);
    _exit(0);
}

//segnale il quale in base ad uno switch permette di effettuare la scelta corretta sul prossimo segnale, e quindi come comportarsi in un determinato evento 
void casi_client_handler (int signum) {
    struct mymsgi caso;
    msgrcv(msqid, &caso, intSize, 15, IPC_NOWAIT);

    mich = caso.mint;
    pid_t pidPersonale = getpid();
    switch(mich) {
        //case 1: chiusura del server a causa del doppio CTRL+C
        case 1:
            if (signal(SIGUSR1, chiusura_server_handler) == SIG_ERR)
                errExit("signal failed");
            kill(pidPersonale, SIGUSR1);
            break;
        //case 2: vittoria perché scaduto il tempo al secondo giocatore
        case 2:
            if (signal(SIGUSR1, vittoria_tempo_handler) == SIG_ERR)
                errExit("signal failed");
            kill(pidPersonale, SIGUSR1);
            break;
        //case 3: vittoria per abbandono da parte del secondo giocatore
        case 3: 
            if (signal(SIGUSR1, vittoria_abbandono_handler) == SIG_ERR)
                errExit("signal failed");
            kill(pidPersonale, SIGUSR1);
            break;
        //segnale per indicare la vittoria della partita
        case 4:
            if (signal(SIGUSR1, vittoria_handler) == SIG_ERR)
                errExit("signal failed");
            kill(pidPersonale, SIGUSR1);
            break;
        //segnale per indicare la sconfitta della partita
        case 5:
            if (signal(SIGUSR1, sconfitta_handler) == SIG_ERR)
                errExit("signal failed");
            kill(pidPersonale, SIGUSR1);
            break;
        //default
        default:
            printf("\n<CLIENT> C'è stato un problema. Ci scusiamo per il disguido\n");
            exit(0);
            break;
    };
};

//segnale per gestire il CTRL+C da parte di un giocatore. In base alla flag momento si comporta in diverso modo
//se è 0: vuol dire che il secondo giocatore deve ancora essere trovato, perciò comunicherà al server che sta abbandonando e quindi di chiudere l'attuale matchmaking
//se è 1: vuol dire che il secondo giocatore è stato trovato, perciò deve comunicare al server che ha abbandonato e di dare vittoria al secondo giocatore
void finegiocatore_handler (int signum) {
    pid_t pidPersonale = getpid();
    struct mymsgi pid = {16, pidPersonale};
    msgsnd(msqid, &pid, intSize, 0);
    if (momento == 0) {
        printf("\n<CLIENT> Chiusura effettuata\n");
        if (pidServer > 0)
            kill(pidServer, SIGUSR1);
        _exit(0);
    } else {
        printf("\n<CLIENT> Hai perso per abbandono\n"
               "<CLIENT> Grazie per aver giocato\n");
        if (pidServer > 0)
            kill(pidServer, SIGUSR1);
        _exit(0);
    }
}

//segnale che scaduto il timer dato dall'alarm porterà alla sconfitta per inattività del giocatore
void timer_handler(int signum) {
    printf("\n<CLIENT> Hai perso per inattività\n");
    printf("<CLIENT> Grazie per aver giocato\n");
    pid_t pidPersonale = getpid();
    struct mymsgi pid = {16, pidPersonale};
    msgsnd(msqid, &pid, intSize, 0);
    kill(pidServer, SIGUSR2);
    _exit(0);
}

//segnale che gestisce un probabile errExit
void errExit_client (int signum) {
    if (pidServer > 0) {
        kill(pidServer, SIGTERM);
        pause();
    } else {
        exit(0);
    }
}

//segnali server

//segnale per la chiusura di tutte le IPC (semafori, messagequeue, shared memory)
void chiusura_totale_handler (int signum) {
    if (msqid >= 0)
        msgctl(msqid, IPC_RMID, NULL);
    if (semid >= 0)
        semctl(semid, 0, IPC_RMID, arg);
    if (shmid >= 0)
        remove_shared_memory(shmid);
    printf("\n<SERVER> Chiusura completata\n");
    _exit(0);
}

//segnale ausiliario per inviare al secondo giocatore la notifica che il server è stato chiuso dopo il doppio CTRL+C
void invio_secondo_giocatore_handler(int signum) {
    struct mymsgi caso = {15, 1};
    msgsnd(msqid, &caso, intSize, 0);
    kill(pidGiocatore2, SIGUSR1);          
    pid_t pid = getpid();
    if (signal(SIGUSR1,chiusura_totale_handler) == SIG_ERR)
        errExit("signal failed");
}

//segnale che gestisce il secondo CTRL+C. Utilizza la flag fase per decidere quale caso gestire
void fineserver_handler2 (int signum) {
    printf("\n<SERVER> Terminazione programma\n");
    //1 per chiusura server
    struct mymsgi caso = {15, 1};
    switch(fase) {
        //case 0: non è ancora stato trovato il primo giocatore, perciò chiude tutto senza problemi
        case 0:
            if (signal(SIGALRM,chiusura_totale_handler) == SIG_ERR)
                errExit("signal failed");
            alarm(1);
            break;
        //case 1: è stato trovato solo il primo giocatore, perciò deve comunicare solo a quest'ultimo della chiusura. Infine chiude tutto
        case 1:
            if (msgsnd(msqid, &caso, intSize, 0) == -1)
                errExit("<SERVER> msgsnd failed\n");
            if (signal(SIGUSR1, SIG_IGN) == SIG_ERR)
                errExit("signal failed");
            kill(pidGiocatore1, SIGUSR1);
            
            if (signal(SIGALRM,chiusura_totale_handler) == SIG_ERR)
                errExit("signal failed");
            alarm(1);
            break;
        //case 2: ci sono tutti e 2 i giocatori, perciò comunicherà al primo giocatore ed effettua una signal in attesa di ricevere risposta dal primo giocatore così da gestire il secondo giocatore
        //nel caso di partita contro la CPU, si comporta come se ci fosse un solo giocatore quindi come se fosse nel case 1
        case 2:
            msgsnd(msqid, &caso, intSize, 0);
            if (scelta != '*') {
                if (signal(SIGUSR1, invio_secondo_giocatore_handler) == SIG_ERR)
                    errExit("signal failed");
                kill(pidGiocatore1, SIGUSR1);
            } else {
                if (signal(SIGUSR1, SIG_IGN) == SIG_ERR)
                    errExit("signal failed");
                kill(pidGiocatore1, SIGUSR1);
                
                if (signal(SIGALRM,chiusura_totale_handler) == SIG_ERR)
                    errExit("signal failed"); 
                alarm(1);
            }
            
            break;
        //default: non fare nulla
        default:
            break;
    }
}

//segnale che gestisce il primo CTRL+C
//in base alla risposta data "resetta" il CTRL+C altrimenti col secondo CTRL+C avverrà la chiusura del server
void fineserver_handler1 (int signum) {
    char c = ' ';
    char string[500];
    int string_length = 0;

    if (signal(SIGINT, SIG_IGN) == SIG_ERR)
        errExit("Signal failed");
    if (signal(SIGINT, fineserver_handler2) == SIG_ERR)
        errExit("signal failed");
    printf("\n<SERVER> Attenzione. E' stato premuto CTRL+C\n"
           "<SERVER> Vuoi terminare il programma? [y/n]\n");
    do {
        scanf("%s", string);
        string_length = strlen(string);
        c = string[0];
        if (c != 'y' && c != 'Y' && c != 'n' && c != 'N' || (string_length != 1))
            printf("Inserire uno dei caratteri suggeriti [y/n]: ");

    } while (c != 'y' && c != 'Y' && c != 'n' && c != 'N' || (string_length != 1));

    if (c == 'y' || c == 'Y') {
        if (signal(SIGINT, fineserver_handler2) == SIG_ERR)
            errExit("signal failed");
        printf("<SERVER> Ok, il prossimo CTRL+C terminerà il programma\n");
    } else {
        if (signal(SIGINT, fineserver_handler1) == SIG_ERR)
            errExit("signal failed");
        printf("<SERVER> Ok, è stato deciso di non terminare\n");
    }
}

//segnale per gestire la sconfitta di un giocatore per inattività
void tempo_scaduto_handler (int signum) {
    printf("\n<SERVER> Un giocatore ha perso per inattività\n");
    //2 client scaduto tempo
    struct mymsgi caso = {15, 2};
    //ricevo il pid del client che ha perso per inattività
    struct mymsgi pid;
    msgrcv(msqid, &pid, intSize, 16, IPC_NOWAIT);
    pid_t pidgiocatore = pid.mint;
    if (pidGiocatore1 == pidgiocatore) {
        msgsnd(msqid, &caso, intSize, 0);
        kill(pidGiocatore2, SIGUSR1);
    }

    if (pidGiocatore2 == pidgiocatore) {
        msgsnd(msqid, &caso, intSize, 0);
        kill(pidGiocatore1, SIGUSR1);
    }
    if (signal(SIGALRM, chiusura_totale_handler) == SIG_ERR)
        errExit("signal failed");
    alarm(1);
};

//segnale che gestisce l'abbandono da parte di un giocatore
void abbandono_handler (int signum) {
    printf("\n<SERVER> Un giocatore ha abbandonato la partita\n");
    //3 giocatore abbandona
    struct mymsgi caso = {15, 3}; 
    //ricevo il pid del client che ha abbandonato
    struct mymsgi pid;
    msgrcv(msqid, &pid, intSize, 16, IPC_NOWAIT);
    pid_t pidgiocatore = pid.mint;

    if (pidGiocatore1 == pidgiocatore && pidGiocatore2 > 0) {
        msgsnd(msqid, &caso, intSize, 0);
        kill(pidGiocatore2, SIGUSR1);
    }

    if (pidGiocatore2 == pidgiocatore && pidGiocatore1 > 0) {
        msgsnd(msqid, &caso, intSize, 0);
        kill(pidGiocatore1, SIGUSR1);
    }

    if (signal(SIGALRM, chiusura_totale_handler) == SIG_ERR)
        errExit("signal failed");
    alarm(1);
};

//segnale che gestice la parità di una partita
void pari_server (int signum) {
    printf("\n<SERVER> La partita e' terminata in parita'\n");
    kill(pidGiocatore1, SIGUSR2);
    if (scelta != '*') {
        kill(pidGiocatore2, SIGUSR2);
    }
}

//segnale che gestisce un probabile errExit
void errExit_server (int signum) {
    if (pidGiocatore1 > 0)
        kill(pidGiocatore1, SIGUSR1);
    if (pidGiocatore2 > 0)
        kill(pidGiocatore2, SIGUSR1);
    
    signal(SIGALRM, chiusura_totale_handler);
    alarm(1);

}

/************************************
*Matricola VR471881, VR474454, 473647
*Nome e Cognome Simone Rossato, Matteo Mantovani, Thomas Lonardi
*Data di realizzazione 5 settembre 2023
*************************************/
