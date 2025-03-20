#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

/*------------------------ STRUCTS ------------------------*/
struct no
{
    int numero_chaves;
    char eh_folha;
    char lotado;
    long int posicao_arq_binario;
    int *filhos, *chaves, *valores;
    int pai_offset;
};

struct __arvoreB
{
    int ordem;
    long int numero_nos;
    int tam_byte_node;
    FILE *arq_binario;
};

/*--------------------Nó-----------------------*/
// cria nó
No *criaNo(ArvoreB *arv)
{
    No *no = malloc(sizeof(No)); // Corrigido: aloca corretamente a estrutura No
    no->numero_chaves = 0;
    no->filhos = malloc(arv->ordem * sizeof(int));
    no->chaves = malloc(arv->ordem * sizeof(int));
    no->valores = malloc(arv->ordem * sizeof(int));
    for (int i = 0; i < arv->ordem; i++)
    {
        no->filhos[i] = INT_MAX;
        no->chaves[i] = INT_MAX;
        no->valores[i] = INT_MAX;
    }
    no->eh_folha = '1';
    no->pai_offset = -1;
    no->lotado = '0';
    no->posicao_arq_binario = arv->numero_nos;
    arv->numero_nos++;
    return no;
}

/*--------------------Árvore-----------------------*/

// cria árvore
ArvoreB *criaArvoreB(long int ordem, FILE *binario)
{
    ArvoreB *arv = malloc(sizeof(ArvoreB));
    arv->arq_binario = binario;
    arv->tam_byte_node = sizeof(No);
    arv->ordem = ordem;
    arv->numero_nos = 0;
    return arv;
}

ArvoreB *insereArvore(ArvoreB *sentinela, int chave, int valor)
{
    No *aux = disk_read(sentinela, 0);
    while (aux->lotado == '1')
    {
        int achou = 0;
        for (int i = 0; i < aux->numero_chaves; i++)
        {
            if (chave < aux->chaves[i])
            {
                aux = disk_read(sentinela, aux->filhos[i]);
                achou = 1;
                break;
            }
        }
        if (achou == 0) aux = disk_read(sentinela, aux->filhos[aux->numero_chaves]);
    }

    aux->numero_chaves++;

    if (aux->numero_chaves == sentinela->ordem) aux->lotado = '1';

    int i = aux->numero_chaves - 2;
    while (i >= 0 && chave < aux->chaves[i])
    {
        i--;
    }
    i++;

    for (int j = aux->numero_chaves - 1; j > i; j--)
    {
        aux->chaves[j] = aux->chaves[j - 1];
        aux->valores[j] = aux->valores[j - 1];
    }

    aux->chaves[i] = chave;
    aux->valores[i] = valor;

    for (int i = 0; i < aux->numero_chaves; i++)
    {
        printf("%d ", aux->chaves[i]);
    }
    printf("\n");

    disk_write(sentinela, aux);

    if (aux->numero_chaves == sentinela->ordem)
    {
        return divideArvore(aux, sentinela);
    }

    return sentinela;
}

ArvoreB *divideArvore(No *aux, ArvoreB *sentinela)
{
    // dividir a raiz
    if(aux->pai_offset == -1){

    }

    // dividir um no no meio
    else{
        
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*----------------DISK_READ--------------------------DISK_WRITE--------------------*/
No *disk_read(ArvoreB *arvore, long int posicao)
{
    FILE *arquivo_bin = arvore->arq_binario;
    No *node = (No *)malloc(sizeof(No));
    fseek(arquivo_bin, posicao * arvore->tam_byte_node, SEEK_SET);
    fread(&node->numero_chaves, sizeof(int), 1, arquivo_bin);
    fread(&node->eh_folha, sizeof(char), 1, arquivo_bin);
    fread(&node->lotado, sizeof(char), 1, arquivo_bin); // Leitura do campo lotado
    fread(&node->posicao_arq_binario, sizeof(long int), 1, arquivo_bin);
    node->chaves = (int *)malloc(arvore->ordem * sizeof(int));
    node->valores = (int *)malloc((arvore->ordem - 1) * sizeof(int));
    node->filhos = (int *)malloc(arvore->ordem * sizeof(int));
    fread(node->chaves, sizeof(int), arvore->ordem, arquivo_bin);
    fread(node->valores, sizeof(int), arvore->ordem - 1, arquivo_bin);
    fread(node->filhos, sizeof(int), arvore->ordem, arquivo_bin);
    fread(&node->pai_offset, sizeof(long int), 1, arquivo_bin);
    return node;
}

void disk_write(ArvoreB *arvore, No *node)
{
    FILE *arquivo_bin = arvore->arq_binario;
    fseek(arquivo_bin, node->posicao_arq_binario * arvore->tam_byte_node, SEEK_SET);
    fwrite(&node->numero_chaves, sizeof(int), 1, arquivo_bin);
    fwrite(&node->eh_folha, sizeof(char), 1, arquivo_bin);
    fwrite(&node->lotado, sizeof(char), 1, arquivo_bin);
    fwrite(&node->posicao_arq_binario, sizeof(long int), 1, arquivo_bin);
    fwrite(node->chaves, sizeof(int), arvore->ordem, arquivo_bin);
    fwrite(node->valores, sizeof(int), arvore->ordem - 1, arquivo_bin);
    fwrite(node->filhos, sizeof(int), arvore->ordem, arquivo_bin);
    fwrite(&node->pai_offset, sizeof(long int), 1, arquivo_bin);
    fflush(arquivo_bin);
}


/*----------------------------TEST FUNCTIONS----------------------------------------*/
long int getOffset(No *no)
{
    return no->posicao_arq_binario;
}

char getLotado(No *no)
{
    return no->lotado;
}