#include <stdio.h>
#include <stdlib.h>
#include "../include/upscale/upscale.h"
#include "../include/upscale/const_size.h"

void upscale(int **M, int h, int w, int result[CONST_SIZE][CONST_SIZE]);

int main()
{
    int h = 3;
    int w = 3;
    int **M = malloc(h * sizeof(int*));
    for(int i = 0; i < h; i++)
        M[i] = malloc(w * sizeof(int));
    int val = 0;
    for(int i=0;i<h;i++)
    {
        for(int j=0;j<w;j++)
        {
            M[i][j] = val % 2;
            val++;
        }
    }

    // Affichage Input
    printf("====== INPUT ====== (%ix%i)\n", h, w);
    for(int i=0;i<h;i++)
    {
        for(int j=0;j<w;j++)
            printf("%d ", M[i][j]);
        printf("\n");
    }
    printf("\n");

    // Execution Script
    int result[CONST_SIZE][CONST_SIZE];
    upscale(M, h, w, result);

    // Affichage du Résultat
    printf("====== OUTPUT ====== (%ix%i)\n", CONST_SIZE, CONST_SIZE);
    for(int i=0;i<CONST_SIZE;i++)
    {
        for(int j=0;j<CONST_SIZE;j++)
            printf("%d ", result[i][j]);
        printf("\n");
    }

    // Free
    for(int i=0;i<h;i++)
        free(M[i]);
    free(M);

    return 0;
}