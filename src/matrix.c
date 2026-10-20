#include <stdio.h>
#include "matrix.h"

//trasforma un array in una matrice
void array_to_matrix(char *array, char **matrice, int riga, int colonna){
    int k = 0;
    for(int i = 0; i < riga; i++){
        for(int j = 0; j < colonna; j++, k++){
            matrice[i][j] = array[k];
        }
    }
}

//stampa la matrice
void stampa_matrix(char **matrice, int riga, int colonna){
    for (int i = 1; i <= colonna; i++)
        printf(" %i ", i);
    printf("\n");
    for(int i = 0; i < riga; i++){
        for(int j = 0; j < colonna; j++){
            printf("|%c|", matrice[i][j]);
        }
        printf("\n");
    }
}

//trasforma una matrice in un array
void matrix_to_array(char* array, char **matrice, int riga, int colonna){
    for(int i = 0, k = 0; i < riga; i++){
        for(int j = 0; j < colonna; j++, k++){
            array[k] = matrice[i][j];
        }
    }
}

/************************************
*Matricola VR471881, VR474454, 473647
*Nome e Cognome Simone Rossato, Matteo Mantovani, Thomas Lonardi
*Data di realizzazione 4 luglio 2023
*************************************/

