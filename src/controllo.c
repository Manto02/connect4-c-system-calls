#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/signal.h>

#include "controllo.h"

//effettua il controllo orizzontale
//tramite due cicli for per ogni riga controlliamo se ci sono 4 valori in sequenza con lo stesso valore, se è così cambia l'array fine, dove fine[0] indica la vittoria e fine[1] la parità
void controllo_orizzontale(char **matrice, int riga, int colonna, int *fine, char coin) {
    int i,j,k;
    int flag;
    int count;
    for(i = 0; i < riga; i++) {
        for(flag = 0, j = 0; j < colonna-4+1 && !flag; j++) {
            for (k = 0, count = 0; k < 4;k++) {
                if (matrice[i][j+k] == coin)
                    count++;
            }
            if (count == 4) {
                flag = 1;
                fine[0] = 1;
                fine[1] = 0;
            }
        }
    }
}

//effettua il controllo verticale
//tramite due cicli for per ogni colonna controlliamo se ci sono 4 valori in sequenza con lo stesso valore, se è così cambia l'array fine, dove fine[0] indica la vittoria e fine[1] la parità
void controllo_verticale(char **matrice, int riga, int colonna, int *fine, char coin) {
    int i,j,k;
    int count, flag;

    for(i = 0; i < colonna; i++) {
        for(flag = 0, j = 0; j < riga-4+1 && !flag; j++) {
            for (k = 0, count = 0; k < 4;k++) {
                if (matrice[j+k][i] == coin)
                    count++;
            }
            if (count == 4) {
                flag = 1;
                fine[0] = 1;
                fine[1] = 0;
            }
        }
    }
}

//effettua il controllo sulla parità
//controlla se la matrice è piena, se lo è abbiamo parità e quindi imposta fine[1] che indica la parità a 1, altrimenti lo imposta a 0

void controllo_pari(char **matrice, int riga, int colonna, int *fine) {
    int i,j;
    int pari = 1;

    for(i = 0; i < riga;i++) {
            for (j = 0; j < colonna && pari; j++)
            if(matrice[i][j] == ' ') {
                pari = 0;
                fine[1] = 0;
            }
        }
    if (pari)
        fine[1] = 1;
}

//effettua il controllo diagonale andando
void controllo_diagonale(char **matrice, int riga, int colonna, int *fine, char coin) {
    int i,j, k;
    int count, flag;

//diagonale principale e sue parallele
    for(i = riga - 1; i >= 0; i--) { 
        for (count = 0, flag = 0, j = colonna - 1, k = i; j >= 0 && k >= 0 && !flag; j--, k--){
            if (matrice[k][j] == coin)
                count++;
            if (matrice[k][j] != coin)
                count = 0;
            if (count == 4) {
                flag = 1;
                fine[0] = 1;
                fine[1] = 0;
            }
        }
    }

    for(i = 0; i < riga; i++) {
        for (count = 0, flag = 0, j = 0, k = i; j < colonna && k < riga && !flag; j++, k++){
            if (matrice[k][j] == coin)
                count++;
            if (matrice[k][j] != coin)
                count = 0;
            if (count == 4) {
                flag = 1;
                fine[0] = 1;
                fine[1] = 0;
            }
        }
    }



        
//diagonale secondaria e sue parallele
    for(i = 0; i < riga; i++) {
        for (count = 0, flag = 0, j = colonna-1, k = i; j >= 0 && k < riga && !flag; j--, k++){
            if (matrice[k][j] == coin)
                count++;
            if (matrice[k][j] != coin)
                count = 0;
            if (count == 4) {
                flag = 1;
                fine[0] = 1;
                fine[1] = 0;
            }
        }
    }

    for(i = riga - 1; i >= 0; i--) {
        for (count = 0, flag = 0, j = 0, k = i; j < colonna && k >= 0 && !flag; j++, k--){
            if (matrice[k][j] == coin)
                count++;
            if (matrice[k][j] != coin)
                count = 0;
            if (count == 4) {
                flag = 1;
                fine[0] = 1;
                fine[1] = 0;
            }
        }
    }
}

//effettua l'inserimento manuale nel gettone andando a chiedere la posizione, controlla se è piena e se non lo è inserisce il gettone nella prima posizione libera della colonna selezionata
void inserimento_manuale_gettone (char ** matrice, int riga, int colonna, char *gettone, int tempo) {
    char c[500];
    int choice;
    int i;
    int flag = 0;

    do {
        alarm(tempo);
        printf("In quale colonna (1-%i) si vuole inserire il gettone?", colonna);
        fgets(c, sizeof(c), stdin);
        alarm(0);
        choice = atoi(c);
        choice--;
        
        if(choice < 0 || choice >= colonna )
            printf("valore non valido reiserire\n");
        else {
            if (matrice[0][choice] != ' ') {
                printf("La colonna e' piena\n");
                choice = -1;
            }
        }
    } while (choice >= colonna || choice < 0);

    for (i = 0; i < riga-1; i++);
    while (!flag) {
        if (matrice[i][choice] == ' ') {
            matrice[i][choice] = gettone[0];
            flag = 1;
        } else {
            i--;
        }
    }
}

//effettua l'inserimento casuale del gettone, si comporta esattamente come per quello manuale
void inserimento_casuale_gettone (char **matrice, int riga, int colonna, char coin) {
    int choice;
    int i;
    int flag = 0;

    do {
        choice = rand() % colonna;
        if (matrice[0][choice] != ' ') {
            choice = -1;
        }
    } while (choice >= colonna || choice < 0);

    for (i = 0; i < riga-1; i++);


    while (!flag) {
        if (matrice[i][choice] == ' ') {
            matrice[i][choice] = coin;
            flag = 1;
        } else {
            i--;
        }
    }
}

/************************************
*Matricola VR471881, VR474454, 473647
*Nome e Cognome Simone Rossato, Matteo Mantovani, Thomas Lonardi
*Data di realizzazione 4 luglio 2023
*************************************/
