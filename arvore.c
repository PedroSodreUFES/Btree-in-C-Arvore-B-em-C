#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
#include <math.h>

/*------------------------ STRUCTS ------------------------*/
struct no
{
    int numero_chaves;
    char eh_folha;
    char lotado;
    int posicao_arq_binario;
    int pai_offset;
    int *filhos, *chaves, *valores;
};

struct __arvoreB
{
    int ordem;
    int numero_nos;
    int tam_byte_node;
    FILE *arq_binario;
    int offsetRaiz;
};

/*--------------------Nó-----------------------*/

No *criaNo(ArvoreB *arv)
{
    No *no = calloc(1, sizeof(No));
    no->eh_folha = '1';
    no->lotado = '0';
    no->numero_chaves = 0;
    no->pai_offset = -1;
    int posicao = arv->numero_nos;
    no->posicao_arq_binario = posicao;
    arv->numero_nos = arv->numero_nos + 1;
    int ordem = arv->ordem, i;
    no->chaves = calloc(ordem, sizeof(int));
    no->filhos = calloc(ordem + 1, sizeof(int));
    no->valores = calloc(ordem, sizeof(int));
    return no;
}

No *criaNoVazio(ArvoreB *arv)
{
    No *no = (No *)malloc(1 * sizeof(No));
    no->eh_folha = '1';
    no->lotado = '0';
    no->numero_chaves = 0;
    no->pai_offset = -1;
    no->posicao_arq_binario = -1;
    int ordem = arv->ordem;
    int *chaves = (int *)malloc(ordem * sizeof(int)), *filhos = (int *)malloc((ordem + 1) * sizeof(int)), *valores = (int *)malloc(ordem * sizeof(int));
    no->chaves = chaves;
    no->filhos = filhos;
    no->valores = valores;
    return no;
}

void liberaNo(No *no)
{
    if (no != NULL)
    {
        if (no->filhos != NULL)
        {
            free(no->filhos);
        }
        if (no->chaves != NULL)
        {
            free(no->chaves);
        }
        if (no->valores != NULL)
        {
            free(no->valores);
        }
        free(no);
    }
}

/*--------------------Árvore-----------------------*/

// cria árvore
ArvoreB *criaArvoreB(int ordem, FILE *binario)
{
    ArvoreB *arv = calloc(1, sizeof(ArvoreB));
    arv->arq_binario = binario;
    arv->tam_byte_node = sizeof(No) + 3 * sizeof(int) * ordem + 1 * sizeof(int);
    arv->ordem = ordem;
    arv->numero_nos = 0;
    arv->offsetRaiz = 0;
    return arv;
}

int retornaOffsetRaiz(ArvoreB *arv)
{
    return arv->offsetRaiz;
}

ArvoreB *insereArvore(ArvoreB *sentinela, int chave, int valor)
{
    No *aux = disk_read(sentinela, 0);
    int i, offset;
    while (aux->eh_folha == '0')
    {
        int achou = 0;
        for (i = 0; i < aux->numero_chaves; i++)
        {
            if (chave < aux->chaves[i])
            {
                offset = aux->filhos[i];
                liberaNo(aux);
                aux = disk_read(sentinela, offset);
                achou = 1;
                break;
            }
            else if (chave == aux->chaves[i])
            {
                aux->valores[i] = valor;
                disk_write(sentinela, aux);
                liberaNo(aux);
                return sentinela;
            }
        }
        if (achou == 0)
        {
            offset = aux->filhos[aux->numero_chaves];
            liberaNo(aux);
            aux = disk_read(sentinela, offset);
        }
    }

    // aumenta o numero de chaves na folha
    aux->numero_chaves++;
    if (aux->numero_chaves == sentinela->ordem - 1)
        aux->lotado = '1';

    // insere de forma ordenada na folha
    i = aux->numero_chaves - 2;
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
        return divideArvore(offset, sentinela);
    }

    // se nao, retorna a sentinela
    liberaNo(aux);
    return sentinela;
}

ArvoreB *divideArvore(int offset, ArvoreB *sentinela)
{
    No *antiga_raiz = disk_read(sentinela, offset);

    // raiz ta lotada, ou seja, pai não existe.
    if (antiga_raiz->pai_offset == -1)
    {
        int i, k;

        // cria o nível acima
        int chave_do_meio = antiga_raiz->chaves[(int)ceil((sentinela->ordem / (double)2)) - 1];
        int valor_do_meio = antiga_raiz->valores[(int)ceil((sentinela->ordem / (double)2)) - 1];
        No *filhoMenor = criaNo(sentinela);
        filhoMenor->numero_chaves = (int)ceil(sentinela->ordem / (double)2 - 1);

        for (i = 0; i < (int)ceil((sentinela->ordem / (double)2)) - 1; i++)
        {
            filhoMenor->chaves[i] = antiga_raiz->chaves[i];
            filhoMenor->valores[i] = antiga_raiz->valores[i];
            filhoMenor->filhos[i] = antiga_raiz->filhos[i];
        }
        filhoMenor->filhos[i] = antiga_raiz->filhos[i];
        filhoMenor->eh_folha = antiga_raiz->eh_folha;
        filhoMenor->lotado = '0';
        filhoMenor->pai_offset = 0;

        if (filhoMenor->eh_folha == '0')
        {
            for (i = 0; i < filhoMenor->numero_chaves + 1; i++)
            {
                No *aux = disk_read(sentinela, filhoMenor->filhos[i]);
                aux->pai_offset = filhoMenor->posicao_arq_binario;
                disk_write(sentinela, aux);
                liberaNo(aux);
            }
        }

        No *filhoMaior = criaNo(sentinela);
        filhoMaior->numero_chaves = sentinela->ordem - (int)ceil((sentinela->ordem / (double)2));
        for (i = 0, k = (int)ceil(sentinela->ordem / (double)2); k < antiga_raiz->numero_chaves; k++, i++)
        {
            filhoMaior->chaves[i] = antiga_raiz->chaves[k];
            filhoMaior->valores[i] = antiga_raiz->valores[k];
            filhoMaior->filhos[i] = antiga_raiz->filhos[k];
        }
        filhoMaior->filhos[i] = antiga_raiz->filhos[k];
        filhoMaior->eh_folha = antiga_raiz->eh_folha;
        filhoMaior->lotado = '0';
        filhoMaior->pai_offset = 0;

        if (filhoMaior->eh_folha == '0')
        {
            for (i = 0; i < filhoMaior->numero_chaves + 1; i++)
            {
                No *aux = disk_read(sentinela, filhoMaior->filhos[i]);
                aux->pai_offset = filhoMaior->posicao_arq_binario;
                disk_write(sentinela, aux);
                liberaNo(aux);
            }
        }

        antiga_raiz->numero_chaves = 1;
        antiga_raiz->filhos[0] = filhoMenor->posicao_arq_binario;
        antiga_raiz->filhos[1] = filhoMaior->posicao_arq_binario;
        antiga_raiz->chaves[0] = chave_do_meio;
        antiga_raiz->valores[0] = valor_do_meio;
        antiga_raiz->eh_folha = '0';
        antiga_raiz->lotado = '0';

        filhoMaior->pai_offset = antiga_raiz->posicao_arq_binario;
        filhoMenor->pai_offset = antiga_raiz->posicao_arq_binario;

        disk_write(sentinela, filhoMenor);
        disk_write(sentinela, filhoMaior);
        disk_write(sentinela, antiga_raiz);

        liberaNo(antiga_raiz);
        liberaNo(filhoMenor);
        liberaNo(filhoMaior);

        return sentinela;
    }

    // ao dividir a raíz, será colocado um nó em um pai que já existe
    else
    {
        int i, k;
        // recupera as chaves do meio
        int chave_do_meio = antiga_raiz->chaves[(int)ceil((sentinela->ordem / (double)2)) - 1];
        int valor_do_meio = antiga_raiz->valores[(int)ceil((sentinela->ordem / (double)2)) - 1];

        No *filhoMenor = criaNo(sentinela);
        filhoMenor->numero_chaves = (int)ceil(sentinela->ordem / (double)2) - 1;
        for (i = 0; i < (int)ceil((sentinela->ordem / (double)2)) - 1; i++)
        {
            filhoMenor->chaves[i] = antiga_raiz->chaves[i];
            filhoMenor->valores[i] = antiga_raiz->valores[i];
            filhoMenor->filhos[i] = antiga_raiz->filhos[i];
        }
        filhoMenor->filhos[i] = antiga_raiz->filhos[i];
        filhoMenor->eh_folha = antiga_raiz->eh_folha;
        filhoMenor->lotado = '0';
        filhoMenor->pai_offset = antiga_raiz->pai_offset;

        if (filhoMenor->eh_folha == '0')
        {
            for (i = 0; i < filhoMenor->numero_chaves + 1; i++)
            {
                No *aux = disk_read(sentinela, filhoMenor->filhos[i]);
                aux->pai_offset = filhoMenor->posicao_arq_binario;
                disk_write(sentinela, aux);
                liberaNo(aux);
            }
        }

        // cria o nivel com a segunda metade:
        No *filhoMaior = criaNo(sentinela);
        filhoMaior->numero_chaves = sentinela->ordem - (int)ceil(sentinela->ordem / (double)2);
        for (i = 0, k = (int)ceil(sentinela->ordem / (double)2); k < antiga_raiz->numero_chaves; k++, i++)
        {
            filhoMaior->chaves[i] = antiga_raiz->chaves[k];
            filhoMaior->valores[i] = antiga_raiz->valores[k];
            filhoMaior->filhos[i] = antiga_raiz->filhos[k];
        }
        filhoMaior->filhos[i] = antiga_raiz->filhos[k];
        filhoMaior->eh_folha = antiga_raiz->eh_folha;
        filhoMaior->lotado = '0';
        filhoMaior->pai_offset = antiga_raiz->posicao_arq_binario;

        if (filhoMaior->eh_folha == '0')
        {
            for (i = 0; i < filhoMaior->numero_chaves + 1; i++)
            {
                No *aux = disk_read(sentinela, filhoMaior->filhos[i]);
                aux->pai_offset = filhoMaior->posicao_arq_binario;
                disk_write(sentinela, aux);
                liberaNo(aux);
            }
        }

        // antiga raiz deve ir ao pai;
        int salvaOffset = antiga_raiz->pai_offset;
        No *pai = disk_read(sentinela, salvaOffset);
        pai->numero_chaves++;
        if (pai->numero_chaves == sentinela->ordem - 1)
            pai->lotado = '1';

        int j;
        i = pai->numero_chaves - 2;
        while (i >= 0 && chave_do_meio < pai->chaves[i])
        {
            i--;
        }
        i++;

        for (j = pai->numero_chaves - 1; j > i; j--)
        {
            pai->chaves[j] = pai->chaves[j - 1];
            pai->valores[j] = pai->valores[j - 1];
            pai->filhos[j + 1] = pai->filhos[j];
        }

        pai->chaves[i] = chave_do_meio;
        pai->valores[i] = valor_do_meio;
        pai->filhos[i] = filhoMenor->posicao_arq_binario;
        pai->filhos[i + 1] = filhoMaior->posicao_arq_binario;
        pai->eh_folha = '0';

        filhoMaior->pai_offset = antiga_raiz->pai_offset;
        filhoMenor->pai_offset = antiga_raiz->pai_offset;
        pai->posicao_arq_binario = salvaOffset;

        disk_write(sentinela, filhoMenor);
        disk_write(sentinela, filhoMaior);
        disk_write(sentinela, antiga_raiz);
        disk_write(sentinela, pai);

        liberaNo(filhoMaior);
        liberaNo(filhoMenor);
        liberaNo(antiga_raiz);

        if (pai->numero_chaves == sentinela->ordem)
        {
            liberaNo(pai);
            return divideArvore(salvaOffset, sentinela);
        }

        liberaNo(pai);
        return sentinela;
    }
}

void buscaNo(ArvoreB *arv, int chave)
{
    No *aux = disk_read(arv, 0);
    int i, offset;
    while (1)
    {
        int achou = 0;
        for (i = 0; i < aux->numero_chaves; i++)
        {
            if (chave == aux->chaves[i])
            {
                printf("O REGISTRO ESTA NA ARVORE!\n");
                liberaNo(aux);
                return;
            }
        }
        if (aux->eh_folha == '1')
        {
            printf("O REGISTRO NAO ESTA NA ARVORE!\n");
            liberaNo(aux);
            return;
        }
        for (i = 0; i < aux->numero_chaves; i++)
        {
            if (chave < aux->chaves[i])
            {
                offset = aux->filhos[i];
                liberaNo(aux);
                aux = disk_read(arv, offset);
                achou = 1;
                break;
            }
        }
        if (achou == 0)
        {
            offset = aux->filhos[aux->numero_chaves];
            liberaNo(aux);
            aux = disk_read(arv, offset);
        }
    }
}

// se aumentar o prazo é mexer aqui
int retiraChave(ArvoreB *arvore, int chave){
    return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*----------------------------TEST FUNCTIONS----------------------------------------*/
int getOffset(No *no)
{
    return no->posicao_arq_binario;
}

char getLotado(No *no)
{
    return no->lotado;
}

void printaChaves(No *no)
{
    for (int i = 0; i < no->numero_chaves; i++)
    {
        printf("%d ", no->chaves[i]);
    }
    printf("\n");
}

void printaValores(No *no)
{
    for (int i = 0; i < no->numero_chaves; i++)
    {
        printf("%d ", no->valores[i]);
    }
    printf("\n");
}

void printaFilhos(No *no)
{
    for (int i = 0; i < no->numero_chaves + 1; i++)
    {
        printf("%d ", no->filhos[i]);
    }
    printf("\n");
}

/*----------------DISK_READ--------------------------DISK_WRITE--------------------*/
No *disk_read(ArvoreB *arvore, int posicao)
{
    FILE *arquivo_bin = arvore->arq_binario;

    int ordem = arvore->ordem;

    No *node = criaNoVazio(arvore);
    char eh_folha, lotado;
    int paiOffset, posicao_arq_binario, numero_chaves;

    fseek(arquivo_bin, posicao * arvore->tam_byte_node, SEEK_SET);

    fread(&numero_chaves, sizeof(int), 1, arquivo_bin);
    fread(&eh_folha, sizeof(char), 1, arquivo_bin);
    fread(&lotado, sizeof(char), 1, arquivo_bin);
    fread(&posicao_arq_binario, sizeof(int), 1, arquivo_bin);
    fread(&paiOffset, sizeof(int), 1, arquivo_bin);
    node->numero_chaves = numero_chaves;
    node->eh_folha = eh_folha;
    node->lotado = lotado;
    node->posicao_arq_binario = posicao_arq_binario;
    node->pai_offset = paiOffset;

    fread(node->chaves, sizeof(int), ordem, arquivo_bin);
    fread(node->valores, sizeof(int), ordem, arquivo_bin);
    fread(node->filhos, sizeof(int), ordem + 1, arquivo_bin);

    return node;
}

void disk_write(ArvoreB *arvore, No *node)
{
    FILE *arquivo_bin = arvore->arq_binario;
    int ordem = arvore->ordem;
    fseek(arquivo_bin, node->posicao_arq_binario * arvore->tam_byte_node, SEEK_SET);

    fwrite(&node->numero_chaves, sizeof(int), 1, arquivo_bin);
    fwrite(&node->eh_folha, sizeof(char), 1, arquivo_bin);
    fwrite(&node->lotado, sizeof(char), 1, arquivo_bin);
    fwrite(&node->posicao_arq_binario, sizeof(int), 1, arquivo_bin);
    fwrite(&node->pai_offset, sizeof(int), 1, arquivo_bin);

    fwrite(node->chaves, sizeof(int), ordem, arquivo_bin);
    fwrite(node->valores, sizeof(int), ordem, arquivo_bin);
    fwrite(node->filhos, sizeof(int), ordem + 1, arquivo_bin);
    fflush(arquivo_bin);
}

char retornaFolha(No *no)
{
    return no->eh_folha;
}

int *retornaFilhos(No *no)
{
    return no->filhos;
}

int retornaNumChaves(No *no)
{
    return no->numero_chaves;
}

int *retornaChaves(No *no)
{
    return no->chaves;
}