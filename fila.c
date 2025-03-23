#include "fila.h"
#include <stdio.h>
#include <stdlib.h>
#include "arvore.h"

struct fila
{
    int **mat;
    int *colunas;
};

void imprimeFila(ArvoreB *arv)
{
    Fila *fila = malloc(sizeof(Fila));

    // pega a raiz;
    No *no = disk_read(arv, 0);
    if (no == NULL)
    {
        printf("Nó é nulo\n");
        exit(1);
    }

    fila->mat = (int **)malloc(1 *sizeof(int*));
    fila->colunas = malloc(sizeof(int));

    int numero_de_filhos_raiz = retornaNumChaves(no) + 1;

    fila->mat[0] = calloc(numero_de_filhos_raiz, sizeof(int));
    int *aux = retornaFilhos(no);
    
    for (int i = 0; i < numero_de_filhos_raiz; i++)
    {
        fila->mat[0][i] = aux[i];
    }


    int row = 1, coluna = 0, i = 0, flag = 0;
    
    // se a raiz for folha, nem precisa checar nada
    if(retornaFolha(no) == '1') flag=1;
    liberaNo(no);

    No *aux2;

    while (flag == 0)
    {
        coluna = 0;
        for (i = 0; i < numero_de_filhos_raiz; i++)
        {
            // pega cada filho da linha e conta quantos filhos este filho tem
            aux2 = disk_read(arv, fila->mat[row-1][i]);

            // se esse filho for folha, ja adiciona a flag pq eh ultima iteração
            if (retornaFolha(aux2) == '1')
            {
                flag = 1;
            }
            coluna += retornaNumChaves(aux2) + 1;
            liberaNo(aux2);
        }

        // aumenta mais um na linha e da realloc;
        row++;
        fila->mat = realloc(fila->mat, row * sizeof(int*));
        fila->mat[row-1] = malloc(coluna * sizeof(int));
        // adiciona os filhos do no na coluna
        int j = 0, k;
        int soma = 0;
        for (int i = 0; i < numero_de_filhos_raiz; i++)
        {
            j = soma;
            aux2 = disk_read(arv, fila->mat[row - 2][i]);
            soma += retornaNumChaves(aux2) + 1;
            int *filhos = retornaFilhos(aux2);
            for (j, k = 0; j < soma; j++, k++)
            {
                fila->mat[row - 1][j] = filhos[k];
            }
            liberaNo(aux2);
        }
        fila->colunas = realloc(fila->colunas, (row-1) * sizeof(int));
        fila->colunas[row-2] = numero_de_filhos_raiz;
        numero_de_filhos_raiz = coluna;
    }

    // imprime a raiz
    No *raiz = disk_read(arv, 0);
    int *rootKeys = retornaChaves(raiz);
    int numChavesRaiz = retornaNumChaves(raiz);
    printf("\n\n-- ARVORE B\n[");
    for(int a=0 ; a<numChavesRaiz ; a++){
        printf("key: %d, ", rootKeys[a]);
    }
    printf("]\n");
    liberaNo(raiz);

    // imprime o resto
    int j;
    for(i=0 ; i<row-1 ; i++){
        for(j=0 ; j<fila->colunas[i] ; j++){
            printf("[");
            no = disk_read(arv, fila->mat[i][j]);
            rootKeys = retornaChaves(no);
            numChavesRaiz = retornaNumChaves(no);
            for(int k=0; k<numChavesRaiz ; k++){
                printf("key: %d, ", rootKeys[k]);
            }
            liberaNo(no);
            printf("] ");
        }
        printf("\n");
    }
    

    // libera a fila
    if(fila != NULL){
        for(int i = 0; i<row ; i++){
            if(fila->mat[i] != NULL) free(fila->mat[i]);
        }
        if(fila->mat != NULL)free(fila->mat);
        if(fila->colunas != NULL) free(fila->colunas);
        free(fila);
    }
}