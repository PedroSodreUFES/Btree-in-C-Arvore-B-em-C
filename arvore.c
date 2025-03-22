#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

#define MIN_CHAVES(ordem) (ceil(((ordem)/2.0 - 1))) // ceil arredonda para cima
#define MAX_CHAVES(ordem) (ordem - 1)

/*------------------------ Estruturas ------------------------*/

struct no {
    int numero_chaves;
    bool eh_folha;
    bool lotado;
    int posicao_arq_binario;
    int pai_offset;
    int* filhos;
    int* chaves;
    int* valores;
};

struct arvoreB {
    int ordem;
    int numero_nos;
    int tam_byte_node;
    FILE* arq_binario;
    int offsetRaiz;
};

/*------------------------ Declarações ------------------------*/
static No* criaNo(ArvoreB* arvore);
static No* criaNoVazio(ArvoreB* arvore);
static void liberaNo(No* no);

static ArvoreB* divideArvore(int offset, ArvoreB* arvore);
static int percorreNo(No* no, int chave);

static ArvoreB* removeChave(ArvoreB* arvore, No* no, int chave);
// static void removeDaFolha(No* no, int indice);
// static void removeDeNoInterno(ArvoreB* arvore, No* no, int indice);
static ArvoreB *garanteMinimo(ArvoreB *sentinela, No *pai, int idx);
static ArvoreB* fundeNos(ArvoreB* arvore, No* pai, int indice);
static ArvoreB* trataUnderflow(ArvoreB* arvore, No* no);
static int getPredecessor(ArvoreB* arvore, No* no);
static int getSucessor(ArvoreB* arvore, No* no);

/*------------------------ Implementação Árvore ------------------------*/
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

ArvoreB *insereArvore(ArvoreB *sentinela, int chave, int valor)
{
    if (sentinela->numero_nos == 0)
    {
        No *raiz = criaNo(sentinela);
        raiz->chaves[0] = chave;
        raiz->valores[0] = valor;
        raiz->numero_chaves = 1;
        disk_write(sentinela, raiz);
        sentinela->offsetRaiz = raiz->posicao_arq_binario;
        return sentinela;
    }
    No *aux = disk_read(sentinela, 0);
    int i, offset;
    while (!aux->eh_folha)
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
        aux->lotado = true;

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

int buscaArvore(ArvoreB *arvore, int chave) {
    No* no = disk_read(arvore, arvore->offsetRaiz);
    while (true) {
        int indice = percorreNo (no, chave);
        if (no->chaves[indice] == chave)
            return no->valores[indice];
        if (no->eh_folha)
            break;
        no = disk_read(arvore, no->filhos[indice]);
    }
    return -1; // Não encontrado
}

ArvoreB* retiraArvore(ArvoreB *sentinela, int chave) {
    if (sentinela->numero_nos == 0) 
        return NULL;

    No* raiz = disk_read(sentinela, sentinela->offsetRaiz);
    sentinela = removeChave(sentinela, raiz, chave);
        
    disk_write(sentinela, raiz); // Atualiza a raiz se não foi substituída
    liberaNo(raiz);
    return sentinela;
}

/*------------------------ Funções de Nó ------------------------*/
No *criaNo(ArvoreB *arv)
{
    No *no = calloc(1, sizeof(No));
    no->numero_chaves = 0;
    no->eh_folha = true;
    no->lotado = false;
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
    no->eh_folha = true;
    no->lotado = false;
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

static void liberaNo(No* no) {
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

/*------------------------ Operações de Disco ------------------------*/

No *disk_read(ArvoreB *arvore, int posicao)
{
    FILE *arquivo_bin = arvore->arq_binario;

    int ordem = arvore->ordem;

    No *node = criaNoVazio(arvore);
    char eh_folha, lotado;
    int paiOffset, posicao_arq_binario, numero_chaves;

    fseek(arquivo_bin, posicao * arvore->tam_byte_node, SEEK_SET);

    fread(&numero_chaves, sizeof(int), 1, arquivo_bin);
    fread(&eh_folha, sizeof(bool), 1, arquivo_bin);
    fread(&lotado, sizeof(bool), 1, arquivo_bin);
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
    fwrite(&node->eh_folha, sizeof(bool), 1, arquivo_bin);
    fwrite(&node->lotado, sizeof(bool), 1, arquivo_bin);
    fwrite(&node->posicao_arq_binario, sizeof(int), 1, arquivo_bin);
    fwrite(&node->pai_offset, sizeof(int), 1, arquivo_bin);

    fwrite(node->chaves, sizeof(int), ordem, arquivo_bin);
    fwrite(node->valores, sizeof(int), ordem, arquivo_bin);
    fwrite(node->filhos, sizeof(int), ordem + 1, arquivo_bin);
    fflush(arquivo_bin);
}

/*------------------------ Funções Inserção --------------------------*/

ArvoreB *divideArvore(int offset, ArvoreB *sentinela)
{
    No *antiga_raiz = disk_read(sentinela, offset);

    // raiz ta lotada, ou seja, pai não existe.
    if (antiga_raiz->pai_offset == -1)
    {
        int i, k;

        // cria o nível acima
        int chave_do_meio = antiga_raiz->chaves[(sentinela->ordem / 2) - 1];
        int valor_do_meio = antiga_raiz->valores[(sentinela->ordem / 2) - 1];
        No *filhoMenor = criaNo(sentinela);
        filhoMenor->numero_chaves = sentinela->ordem / 2 - 1;

        for (i = 0; i < (sentinela->ordem / 2) - 1; i++)
        {
            filhoMenor->chaves[i] = antiga_raiz->chaves[i];
            filhoMenor->valores[i] = antiga_raiz->valores[i];
            filhoMenor->filhos[i] = antiga_raiz->filhos[i];
        }
        filhoMenor->filhos[i] = antiga_raiz->filhos[i];
        filhoMenor->eh_folha = antiga_raiz->eh_folha;
        filhoMenor->lotado = false;
        filhoMenor->pai_offset = 0;

        No *filhoMaior = criaNo(sentinela);
        filhoMaior->numero_chaves = sentinela->ordem - (sentinela->ordem / 2);
        for (i = 0, k = sentinela->ordem / 2; k < antiga_raiz->numero_chaves; k++, i++)
        {
            filhoMaior->chaves[i] = antiga_raiz->chaves[k];
            filhoMaior->valores[i] = antiga_raiz->valores[k];
            filhoMaior->filhos[i] = antiga_raiz->filhos[k];
        }
        filhoMaior->filhos[i] = antiga_raiz->filhos[k];
        filhoMaior->eh_folha = antiga_raiz->eh_folha;
        filhoMaior->lotado = false;
        filhoMaior->pai_offset = 0;

        antiga_raiz->numero_chaves = 1;
        antiga_raiz->filhos[0] = filhoMenor->posicao_arq_binario;
        antiga_raiz->filhos[1] = filhoMaior->posicao_arq_binario;
        antiga_raiz->chaves[0] = chave_do_meio;
        antiga_raiz->valores[0] = valor_do_meio;
        antiga_raiz->eh_folha = false;
        antiga_raiz->lotado = false;

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
        int chave_do_meio = antiga_raiz->chaves[(sentinela->ordem / 2) - 1];
        int valor_do_meio = antiga_raiz->valores[(sentinela->ordem / 2) - 1];

        No *filhoMenor = criaNo(sentinela);
        filhoMenor->numero_chaves = sentinela->ordem / 2 - 1;
        for (i = 0; i < (sentinela->ordem / 2) - 1; i++)
        {
            filhoMenor->chaves[i] = antiga_raiz->chaves[i];
            filhoMenor->valores[i] = antiga_raiz->valores[i];
            filhoMenor->filhos[i] = antiga_raiz->filhos[i];
        }
        filhoMenor->filhos[i] = antiga_raiz->filhos[i];
        filhoMenor->eh_folha = antiga_raiz->eh_folha;
        filhoMenor->lotado = false;
        filhoMenor->pai_offset = antiga_raiz->pai_offset;

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
        filhoMaior->eh_folha = antiga_raiz->eh_folha;
        filhoMaior->lotado = false;
        filhoMaior->pai_offset = antiga_raiz->posicao_arq_binario;

        // antiga raiz deve ir ao pai;
        int salvaOffset = antiga_raiz->pai_offset;
        No *pai = disk_read(sentinela, salvaOffset);
        pai->numero_chaves++;
        if (pai->numero_chaves == sentinela->ordem - 1)
            pai->lotado = true;

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

        pai->eh_folha = false;

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

/*------------------------ Funções Remoção ---------------------------*/
// Função recursiva para remover uma chave
static ArvoreB *removeChave(ArvoreB *sentinela, No *no, int chave) {
    int i = 0;
    while (i < no->numero_chaves && chave > no->chaves[i]) {
        i++;
    }

    // Caso 1: A chave está no nó atual
    if (i < no->numero_chaves && chave == no->chaves[i]) {
        if (no->eh_folha) {
            // Caso 1a: A chave está em um nó folha
            for (int j = i; j < no->numero_chaves - 1; j++) {
                no->chaves[j] = no->chaves[j + 1];
                no->valores[j] = no->valores[j + 1];
            }
            no->numero_chaves--;
            disk_write(sentinela, no);

            if (no->numero_chaves < (sentinela->ordem - 1) / 2) {
                // Tratar underflow
                sentinela = trataUnderflow(sentinela, no);
            }
        } else {
            // Caso 1b: A chave está em um nó interno
            No *filhoEsq = disk_read(sentinela, no->filhos[i]);
            No *filhoDir = disk_read(sentinela, no->filhos[i + 1]);

            if (filhoEsq->numero_chaves > (sentinela->ordem - 1) / 2) {
                // Substituir pelo predecessor
                int pred = getPredecessor(sentinela, filhoEsq);
                no->chaves[i] = pred;
                disk_write(sentinela, no);
                sentinela = removeChave(sentinela, filhoEsq, pred);
            } else if (filhoDir->numero_chaves > (sentinela->ordem - 1) / 2) {
                // Substituir pelo sucessor
                int succ = getSucessor(sentinela, filhoDir);
                no->chaves[i] = succ;
                disk_write(sentinela, no);
                sentinela = removeChave(sentinela, filhoDir, succ);
            } else {
                // Fundir os filhos e remover a chave
                sentinela = fundeNos(sentinela, no, i);
                sentinela = removeChave(sentinela, filhoEsq, chave);
            }
            liberaNo(filhoEsq);
            liberaNo(filhoDir);
        }
    } else {
        // Caso 2: A chave não está no nó atual
        if (no->eh_folha) {
            // A chave não existe na árvore
            return sentinela;
        }

        No *filho = disk_read(sentinela, no->filhos[i]);
        if (filho->numero_chaves == (sentinela->ordem - 1) / 2) {
            // Garantir que o filho tenha pelo menos o mínimo de chaves
            sentinela = garanteMinimo(sentinela, no, i);
            filho = disk_read(sentinela, no->filhos[i]);
        }
        sentinela = removeChave(sentinela, filho, chave);
        liberaNo(filho);
    }

    return sentinela;
}

// Função para tratar underflow em um nó
static ArvoreB *trataUnderflow(ArvoreB *sentinela, No *no) {
    if (no->pai_offset == -1) {
        // Caso especial: nó é a raiz
        if (no->numero_chaves == 0) {
            // A raiz ficou vazia, atualizar a raiz
            sentinela->offsetRaiz = no->filhos[0];
            liberaNo(no);
        }
        return sentinela;
    }

    No *pai = disk_read(sentinela, no->pai_offset);
    int idx = 0;
    while (idx < pai->numero_chaves && pai->filhos[idx] != no->posicao_arq_binario) {
        idx++;
    }

    // Tentar emprestar do irmão esquerdo
    if (idx > 0) {
        No *irmaoEsq = disk_read(sentinela, pai->filhos[idx - 1]);
        if (irmaoEsq->numero_chaves > (sentinela->ordem - 1) / 2) {
            // Emprestar do irmão esquerdo
            for (int i = no->numero_chaves; i > 0; i--) {
                no->chaves[i] = no->chaves[i - 1];
                no->valores[i] = no->valores[i - 1];
            }
            no->chaves[0] = pai->chaves[idx - 1];
            no->valores[0] = pai->valores[idx - 1];
            no->numero_chaves++;

            pai->chaves[idx - 1] = irmaoEsq->chaves[irmaoEsq->numero_chaves - 1];
            pai->valores[idx - 1] = irmaoEsq->valores[irmaoEsq->numero_chaves - 1];
            irmaoEsq->numero_chaves--;

            disk_write(sentinela, no);
            disk_write(sentinela, pai);
            disk_write(sentinela, irmaoEsq);
            liberaNo(irmaoEsq);
            return sentinela;
        }
        liberaNo(irmaoEsq);
    }

    // Tentar emprestar do irmão direito
    if (idx < pai->numero_chaves) {
        No *irmaoDir = disk_read(sentinela, pai->filhos[idx + 1]);
        if (irmaoDir->numero_chaves > (sentinela->ordem - 1) / 2) {
            // Emprestar do irmão direito
            no->chaves[no->numero_chaves] = pai->chaves[idx];
            no->valores[no->numero_chaves] = pai->valores[idx];
            no->numero_chaves++;

            pai->chaves[idx] = irmaoDir->chaves[0];
            pai->valores[idx] = irmaoDir->valores[0];

            for (int i = 0; i < irmaoDir->numero_chaves - 1; i++) {
                irmaoDir->chaves[i] = irmaoDir->chaves[i + 1];
                irmaoDir->valores[i] = irmaoDir->valores[i + 1];
            }
            irmaoDir->numero_chaves--;

            disk_write(sentinela, no);
            disk_write(sentinela, pai);
            disk_write(sentinela, irmaoDir);
            liberaNo(irmaoDir);
            return sentinela;
        }
        liberaNo(irmaoDir);
    }

    // Fundir com um irmão
    if (idx > 0) {
        // Fundir com o irmão esquerdo
        sentinela = fundeNos(sentinela, pai, idx - 1);
    } else {
        // Fundir com o irmão direito
        sentinela = fundeNos(sentinela, pai, idx);
    }

    liberaNo(pai);
    return sentinela;
}

// Função para fundir dois nós
static ArvoreB *fundeNos(ArvoreB *sentinela, No *pai, int idx) {
    No *noEsq = disk_read(sentinela, pai->filhos[idx]);
    No *noDir = disk_read(sentinela, pai->filhos[idx + 1]);

    // Mover a chave do pai para o nó esquerdo
    noEsq->chaves[noEsq->numero_chaves] = pai->chaves[idx];
    noEsq->valores[noEsq->numero_chaves] = pai->valores[idx];
    noEsq->numero_chaves++;

    // Copiar chaves e valores do nó direito para o nó esquerdo
    for (int i = 0; i < noDir->numero_chaves; i++) {
        noEsq->chaves[noEsq->numero_chaves + i] = noDir->chaves[i];
        noEsq->valores[noEsq->numero_chaves + i] = noDir->valores[i];
    }
    noEsq->numero_chaves += noDir->numero_chaves;

    // Atualizar o pai
    for (int i = idx; i < pai->numero_chaves - 1; i++) {
        pai->chaves[i] = pai->chaves[i + 1];
        pai->valores[i] = pai->valores[i + 1];
        pai->filhos[i + 1] = pai->filhos[i + 2];
    }
    pai->numero_chaves--;

    disk_write(sentinela, noEsq);
    disk_write(sentinela, pai);
    disk_write(sentinela, noDir);

    liberaNo(noDir);
    return sentinela;
}

// Função para garantir que um nó tenha pelo menos o mínimo de chaves
static ArvoreB *garanteMinimo(ArvoreB *sentinela, No *pai, int idx) {
    No *no = disk_read(sentinela, pai->filhos[idx]);

    if (no->numero_chaves >= (sentinela->ordem - 1) / 2) {
        // O nó já tem chaves suficientes
        liberaNo(no);
        return sentinela;
    }

    // Tentar emprestar do irmão esquerdo
    if (idx > 0) {
        No *irmaoEsq = disk_read(sentinela, pai->filhos[idx - 1]);
        if (irmaoEsq->numero_chaves > (sentinela->ordem - 1) / 2) {
            // Emprestar do irmão esquerdo
            for (int i = no->numero_chaves; i > 0; i--) {
                no->chaves[i] = no->chaves[i - 1];
                no->valores[i] = no->valores[i - 1];
            }
            no->chaves[0] = pai->chaves[idx - 1];
            no->valores[0] = pai->valores[idx - 1];
            no->numero_chaves++;

            pai->chaves[idx - 1] = irmaoEsq->chaves[irmaoEsq->numero_chaves - 1];
            pai->valores[idx - 1] = irmaoEsq->valores[irmaoEsq->numero_chaves - 1];
            irmaoEsq->numero_chaves--;

            disk_write(sentinela, no);
            disk_write(sentinela, pai);
            disk_write(sentinela, irmaoEsq);
            liberaNo(irmaoEsq);
            liberaNo(no);
            return sentinela;
        }
        liberaNo(irmaoEsq);
    }

    // Tentar emprestar do irmão direito
    if (idx < pai->numero_chaves) {
        No *irmaoDir = disk_read(sentinela, pai->filhos[idx + 1]);
        if (irmaoDir->numero_chaves > (sentinela->ordem - 1) / 2) {
            // Emprestar do irmão direito
            no->chaves[no->numero_chaves] = pai->chaves[idx];
            no->valores[no->numero_chaves] = pai->valores[idx];
            no->numero_chaves++;

            pai->chaves[idx] = irmaoDir->chaves[0];
            pai->valores[idx] = irmaoDir->valores[0];

            for (int i = 0; i < irmaoDir->numero_chaves - 1; i++) {
                irmaoDir->chaves[i] = irmaoDir->chaves[i + 1];
                irmaoDir->valores[i] = irmaoDir->valores[i + 1];
            }
            irmaoDir->numero_chaves--;

            disk_write(sentinela, no);
            disk_write(sentinela, pai);
            disk_write(sentinela, irmaoDir);
            liberaNo(irmaoDir);
            liberaNo(no);
            return sentinela;
        }
        liberaNo(irmaoDir);
    }

    // Fundir com um irmão
    if (idx > 0) {
        // Fundir com o irmão esquerdo
        sentinela = fundeNos(sentinela, pai, idx - 1);
    } else {
        // Fundir com o irmão direito
        sentinela = fundeNos(sentinela, pai, idx);
    }

    liberaNo(no);
    return sentinela;
}

// Função para obter o predecessor de uma chave
static int getPredecessor(ArvoreB *sentinela, No *no) {
    while (!no->eh_folha) {
        no = disk_read(sentinela, no->filhos[no->numero_chaves]);
    }
    return no->chaves[no->numero_chaves - 1];
}

// Função para obter o sucessor de uma chave
static int getSucessor(ArvoreB *sentinela, No *no) {
    while (!no->eh_folha) {
        no = disk_read(sentinela, no->filhos[0]);
    }
    return no->chaves[0];
}

/*------------------------ Funções Auxiliares ------------------------*/
// Retorna o índice da chave no nó
static int percorreNo (No* no, int chave) {
    int i = 0;
    while (chave > no->chaves[i] && i < no->numero_chaves) i++;
    return i;
}

int retornaOffsetRaiz(ArvoreB *arv)
{
    return arv->offsetRaiz;
}


/*-TESTEEEE*/

void imprimeArvore(ArvoreB *arvore, int posicao, int nivel) {
    No *no = disk_read(arvore, posicao);
    
    // Imprime indentação
    for (int i = 0; i < nivel; i++) printf("  ");
    
    // Imprime chaves
    printf("Nó %d (nivel %d): [", posicao, nivel);
    for (int i = 0; i < no->numero_chaves; i++) {
        printf("%d", no->chaves[i]);
        if (i < no->numero_chaves - 1) printf(", ");
    }
    printf("]\n");

    // Imprime filhos recursivamente
    if (!no->eh_folha) {
        for (int i = 0; i <= no->numero_chaves; i++) {
            imprimeArvore(arvore, no->filhos[i], nivel + 1);
        }
    }
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

void printaChaves(No *no)
{
    for (int i = 0; i < no->numero_chaves; i++)
    {
        printf("%d ", no->chaves[i]);
        printf("pao\n");
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

void teste()
{
    FILE *bin = fopen("binario.bin", "w+b");
    ArvoreB *arv = criaArvoreB(5, bin);
    No *no1 = criaNo(arv);
    No *no2 = criaNo(arv);
    no1->eh_folha = 'a';
    no1->lotado = 'b';
    no1->pai_offset = 32;
    no1->posicao_arq_binario = 0;

    no2->numero_chaves = 5;
    no1->numero_chaves = 5;

    for (int i = 0; i < 6; i++)
    {
        no1->filhos[i] = i + 1;
    }

    for (int i = 0; i < 6; i++)
    {
        no2->filhos[i] = i + 3;
    }

    disk_write(arv, no1);
    disk_write(arv, no2);

    liberaNo(no1);
    liberaNo(no2);

    No *no3 = disk_read(arv, 0);
    printaFilhos(no3);
    no3 = disk_read(arv, 1);
    printaFilhos(no3);
    no3 = disk_read(arv, 0);
    printaFilhos(no3);
    printf("\n");
}

// static No* getPredecessor(ArvoreB *arvore, No *no) {
//     No *atual = no;

//     // Desce até o nó folha mais à direita
//     while (!atual->eh_folha) {
//         atual = disk_read(arvore, atual->filhos[atual->numero_chaves]);
//     }
//     return atual;
// }

// static No* getSucessor(ArvoreB *arvore, No *no) {
//     No *atual = no;

//     // Desce até o nó folha mais à esquerda
//     while (!atual->eh_folha) {
//         atual = disk_read(arvore, atual->filhos[0]);
//     }
//     return atual;
// }


// TESTANDO IMPLEMENTACOES DIFERENTES PARA EVITAR VAZAMENTO DE MEMORIA

