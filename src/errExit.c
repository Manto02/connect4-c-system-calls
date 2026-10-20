#include <errExit.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

//short-cut che permette di stampare la stringa che passiamo come parametro seguito da una stringa che indica il possibile errore occorso e infine chiude il processo con il SIGTERM
void errExit(char *msg){
    perror(msg);
    pid_t pid = getpid();
    kill(pid, SIGTERM);
}

/************************************
*Matricola VR471881, VR474454, 473647
*Nome e Cognome Simone Rossato, Matteo Mantovani, Thomas Lonardi
*Data di realizzazione 4 luglio 2023
*************************************/