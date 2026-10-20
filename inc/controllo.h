#ifndef _CONTROLLI_HH
#define _CONTROLLI_HH

#include <stdio.h>
#include <stdlib.h>

#include "shared_memory.h"


void controllo_orizzontale(char **matrice, int riga, int colonna, int *fine, char coin);
void controllo_verticale(char **matrice, int riga, int colonna, int *fine, char coin);
void controllo_diagonale(char **matrice, int riga, int colonna, int *fine, char coin);
void controllo_pari(char **matrice, int riga, int colonna, int *fine);

void inserimento_manuale_gettone(char **matrice, int riga, int colonna, char *gettone, int tempo);
void inserimento_casuale_gettone (char **matrice, int riga, int colonna, char gettone);

#endif