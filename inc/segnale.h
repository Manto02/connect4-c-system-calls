#ifndef _SEGNALI_HH
#define _SEGNALI_HH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "errExit.h"
#include "semaphore.h"
#include "shared_memory.h"

extern pid_t pidServer;
extern pid_t pidGiocatore1;
extern pid_t pidGiocatore2;

extern int momento;
extern int fase;
extern char scelta;
extern int mich;

extern int shmid;
extern int msqid;
extern int semid;

extern size_t carSize;
extern size_t intSize;

extern union semun arg;

struct mymsgc{
    long mtype;
    char mtext[100];
};

struct mymsgi {
    long mtype;
    int mint;
};

struct mymsgpid {
    long mtype;
    pid_t mint;
};

//segnali client

void chiusura_server_handler (int signum);
void pari_client (int signum);
void vittoria_abbandono_handler (int signum);
void vittoria_tempo_handler (int signum);
void vittoria_handler (int signum);
void sconfitta_handler (int signum);
void casi_client_handler (int signum);
void finegiocatore_handler (int signum);
void timer_handler(int signum);
void errExit_client (int signum);

//segnali server

void chiusura_totale_handler (int signum);
void invio_secondo_giocatore_handler(int signum);
void fineserver_handler2 (int signum);
void fineserver_handler1 (int signum);
void tempo_scaduto_handler (int signum);
void abbandono_handler (int signum);
void pari_server (int signum);
void errExit_server (int signum);

#endif