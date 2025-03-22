#include "fila.h"
#include <stdio.h>
#include <stdlib.h>
#include "arvore.h"

struct fila
{
    int **mat;
    int linhas;
};

Fila *criaFila(ArvoreB *arv)
{
    Fila *fila = malloc(sizeof(Fila));

    // pega a raiz;
    No *no = disk_read(arv, 0);
    if (no == NULL)
    {
        printf("Nó é nulo\n");
        exit(1);
    }

    fila->mat = (int **)malloc(1 * sizeof(int *));
    int numero_de_filhos_raiz = retornaNumChaves(no) + 1;
    fila->mat[0] = malloc(numero_de_filhos_raiz * sizeof(int));
    int *aux = retornaFilhos(no);
    for (int i = 0; i < numero_de_filhos_raiz; i++)
    {
        fila->mat[0][i] = aux[i];
    }

    int row = 0, coluna = 0, i = 0, flag = 0;
    No *aux2;

    while (flag == 0)
    {
        coluna = 0;
        for (i = 0; i < numero_de_filhos_raiz; i++)
        {
            printf("passo\n");
            // pega cada filho da linha e conta quantos filhos este filho tem
            aux2 = disk_read(arv, fila->mat[row][i]);

            // se esse filho for folha, ja adiciona a flag pq eh ultima iteração
            if (retornaFolha(no) == '0')
            {
                flag = 1;
            }
            coluna += retornaNumChaves(aux2) + 1;
            printaFilhos(aux2);
            liberaNo(aux2);
        }

        // aumenta mais um na linha e da realloc;
        printf("%d\n", coluna);
        row++;
        fila->mat[row] = malloc(coluna * sizeof(int));

        // adiciona os filhos do no na coluna
        int j = 0, k;
        for (int i = 0; i < numero_de_filhos_raiz; i++)
        {
            aux2 = disk_read(arv, fila->mat[row - 1][i]);
            int *filhos = retornaFilhos(aux2);
            int n = retornaNumChaves(aux2) + 1;
            for (j, k = 0; j < j + n; j++, k++)
            {
                fila->mat[row - 1][j] = filhos[k];
            }
            free(filhos);
            liberaNo(aux2);
        }
    }

    return fila;
}
