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
    int posicao_arq_binario;
    int *filhos, *chaves, *valores;
    int pai_offset;
};

struct __arvoreB
{
    int ordem;
    int numero_nos;
    int tam_byte_node;
    FILE *arq_binario;
};

/*--------------------Nó-----------------------*/
// cria nó
No *criaNo(ArvoreB *arv)
{
    No *no = calloc(1, sizeof(No));
    no->numero_chaves = 0;
    no->eh_folha = '1';
    no->lotado = '0';
    no->pai_offset = -1;
    no->posicao_arq_binario = arv->numero_nos;
    arv->numero_nos++;
    no->chaves = (int *)calloc(arv->ordem, sizeof(int));
    no->valores = (int *)calloc((arv->ordem - 1), sizeof(int));
    no->filhos = (int *)calloc(arv->ordem, sizeof(int));
    for (int i = 0; i < arv->ordem; i++)
    {
        no->filhos[i] = INT_MAX;
        no->chaves[i] = INT_MAX;
        if (i < arv->ordem - 1)
        {
            no->valores[i] = INT_MAX;
        }
    }

    return no;
}

void liberaNo(No *no)
{
    if (no != NULL)
    {
        if (no->filhos != NULL)
            free(no->filhos);
        if (no->chaves != NULL)
            free(no->chaves);
        if (no->valores != NULL)
            free(no->valores);
        free(no);
    }
}

/*--------------------Árvore-----------------------*/

// cria árvore
ArvoreB *criaArvoreB(int ordem, FILE *binario)
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
    // acha a folha
    while (aux->eh_folha == '0')
    {
        int achou = 0;
        for (int i = 0; i < aux->numero_chaves; i++)
        {
            if (chave < aux->chaves[i])
            {
                int offset = aux->filhos[i];
                liberaNo(aux);
                aux = disk_read(sentinela, offset);
                achou = 1;
                break;
            }
            else if(chave == aux->chaves[i]){
                aux->valores[i] = valor;
                disk_write(sentinela, aux);
                liberaNo(aux);
                return sentinela;
            }
        }
        if (achou == 0){aux = disk_read(sentinela, aux->filhos[aux->numero_chaves]);}
    }

    // aumenta o numero de chaves na folha
    aux->numero_chaves++;
    if(aux->numero_chaves == sentinela->ordem-1) aux->lotado = '1';

    // insere de forma ordenada na folha
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

    // escreve o nó na memoria
    disk_write(sentinela, aux);

    // se o nó estiver além da capacidade, divide ele:
    if (aux->numero_chaves == sentinela->ordem)
    {
        int offset = aux->posicao_arq_binario;
        liberaNo(aux);
        printf("Terei de dividir\n");
        return divideArvore(offset, sentinela);
    }

    // se nao, retorna a sentinela
    liberaNo(aux);
    return sentinela;
}

ArvoreB *divideArvore(int offset, ArvoreB *sentinela)
{
    No *antiga_raiz = disk_read(sentinela, offset);

    // raiz ta lotada
    if (antiga_raiz->pai_offset== -1)
    {
        int i, k, contfilhos = 0;

        // cria o nível acima
        int chave_do_meio = antiga_raiz->chaves[(sentinela->ordem / 2) - 1];
        int valor_do_meio = antiga_raiz->valores[(sentinela->ordem/2)-1];
        No *pai = criaNo(sentinela);
        pai->numero_chaves = 1; // isso significa que há 2 filhos também :)
        pai->eh_folha= '0';

        // cria o nivel com a segunda metade:
        No *filhoMaior = criaNo(sentinela);
        filhoMaior->numero_chaves = sentinela->ordem - (sentinela->ordem / 2);
        for (i = 0, k = sentinela->ordem / 2; k < antiga_raiz->numero_chaves; k++, i++)
        {
            filhoMaior->chaves[i] = antiga_raiz->chaves[k];
            filhoMaior->valores[i] = antiga_raiz->valores[k];
            filhoMaior->filhos[i] = antiga_raiz->filhos[k];
        }
        filhoMaior->filhos[i] = antiga_raiz->filhos[k];
        if(filhoMaior->filhos[0] == INT_MAX){
            filhoMaior->eh_folha = '1';
        }
        else{
            filhoMaior->eh_folha = '0';
        }
        filhoMaior->lotado = '0';
        filhoMaior->pai_offset = 0;

        // reajusta a raiz para ser a metade menor
        antiga_raiz->numero_chaves = (sentinela->ordem/2) -1;
        antiga_raiz->lotado = '0';
        antiga_raiz->pai_offset = 0;
        antiga_raiz->posicao_arq_binario = pai->posicao_arq_binario;
        if(antiga_raiz->filhos[0] == INT_MAX){
            antiga_raiz->eh_folha = '1';
        }
        else{
            antiga_raiz->eh_folha = '0';
        }

        // atualiza o pai
        pai->posicao_arq_binario=0;
        pai->filhos[0] = antiga_raiz->posicao_arq_binario;
        pai->filhos[1] = filhoMaior->posicao_arq_binario;
        pai->valores[0] = valor_do_meio;
        pai->chaves[0] = chave_do_meio;
        pai->pai_offset = -1;
        
        disk_write(sentinela, pai);
        disk_write(sentinela, antiga_raiz);
        disk_write(sentinela, filhoMaior);

        return sentinela;
    }

    // no qualquer está lotado
    else
    {

    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*----------------DISK_READ--------------------------DISK_WRITE--------------------*/
No *disk_read(ArvoreB *arvore, int posicao)
{
    FILE *arquivo_bin = arvore->arq_binario;
    No *node = (No *)malloc(sizeof(No));
    fseek(arquivo_bin, posicao * arvore->tam_byte_node, SEEK_SET);
    fread(&node->numero_chaves, sizeof(int), 1, arquivo_bin);
    fread(&node->eh_folha, sizeof(char), 1, arquivo_bin);
    fread(&node->lotado, sizeof(char), 1, arquivo_bin);
    fread(&node->posicao_arq_binario, sizeof(int), 1, arquivo_bin);
    node->chaves = (int *)malloc(arvore->ordem * sizeof(int));
    node->valores = (int *)malloc((arvore->ordem - 1) * sizeof(int));
    node->filhos = (int *)malloc(arvore->ordem * sizeof(int));
    fread(node->chaves, sizeof(int), arvore->ordem, arquivo_bin);
    fread(node->valores, sizeof(int), arvore->ordem - 1, arquivo_bin);
    fread(node->filhos, sizeof(int), arvore->ordem, arquivo_bin);
    fread(&node->pai_offset, sizeof(int), 1, arquivo_bin);
    return node;
}

void disk_write(ArvoreB *arvore, No *node)
{
    FILE *arquivo_bin = arvore->arq_binario;
    fseek(arquivo_bin, node->posicao_arq_binario * arvore->tam_byte_node, SEEK_SET);
    fwrite(&node->numero_chaves, sizeof(int), 1, arquivo_bin);
    fwrite(&node->eh_folha, sizeof(char), 1, arquivo_bin);
    fwrite(&node->lotado, sizeof(char), 1, arquivo_bin);
    fwrite(&node->posicao_arq_binario, sizeof(int), 1, arquivo_bin);
    fwrite(node->chaves, sizeof(int), arvore->ordem, arquivo_bin);
    fwrite(node->valores, sizeof(int), arvore->ordem - 1, arquivo_bin);
    fwrite(node->filhos, sizeof(int), arvore->ordem, arquivo_bin);
    fwrite(&node->pai_offset, sizeof(int), 1, arquivo_bin);
    fflush(arquivo_bin);
}

/*----------------------------TEST FUNCTIONS----------------------------------------*/
int getOffset(No *no)
{
    return no->posicao_arq_binario;
}

char getLotado(No *no)
{
    return no->lotado;
}

void printaChaves(No *no){
    for(int i=0; i< no->numero_chaves ; i++){
        printf("%d ", no->chaves[i]);
    }
    printf("\n");
}