#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>

#define MIN_CHAVES(ordem) (ceil(((ordem)/2 - 1))) // ceil arredonda para cima
#define MAX_CHAVES(ordem) (ordem - 1)

/*------------------------ STRUCTS ------------------------*/
struct no
{
    int numero_chaves;
    bool eh_folha;
    bool lotado;
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


/*------------------------ FUNÇÕES AUXILIARES ------------------------*/
static int percorreNo (No* no, int chave);
static int removeChave(ArvoreB* arvore, No* no, int chave);
static void removeDaFolha(No* no, int indice);
static void removeDeNoInterno(ArvoreB* arvore, No* no, int indice);
static void tratarUnderflow(ArvoreB* arvore, No* pai, int indice);
static bool emprestaDoIrmaoEsquerdo(ArvoreB* arvore, No* pai, int indice);
static bool emprestaDoIrmaoDireito(ArvoreB* arvore, No* pai, int indice);
static No* getPredecessor(ArvoreB *arvore, No *no);
static No* getSucessor(ArvoreB *arvore, No *no);
/*--------------------Nó-----------------------*/
// cria nó
No *criaNo(ArvoreB *arv)
{
    No *no = calloc(1, sizeof(No));
    no->numero_chaves = 0;
    no->eh_folha = true;
    no->lotado = false;
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

static void divideNo (ArvoreB* arvore, No* no) {
}

// Retorna a posição da chave a ser inserida.
static int percorreNo (No* no, int chave) {
    int i = 0;
    while (i < no->numero_chaves && chave > no->chaves[i]) i++;
    return i; // Para uma posição antes da chave maior que a chave a ser inserida
}

void insereNoBinario(ArvoreB *arvore, No *no){
    fseek(arvore->arq_binario, no->posicao_arq_binario * arvore->tam_byte_node, SEEK_SET);
    fwrite(no, arvore->tam_byte_node, 1, arvore->arq_binario);
}

/*------------------------------------------------*/


/*------------------------ FUNÇÕES ÁRVORE ------------------------*/

ArvoreB *insereArvore(ArvoreB *sentinela, int chave, int valor)
{
    No *aux = disk_read(sentinela, 0);
    // acha a folha
    while (!aux->eh_folha)
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

int retiraArvore(ArvoreB *arvore, int chave) {
    if (arvore->numero_nos == 0) return -1;

    No* raiz = disk_read(arvore, 0);
    int registro = removeChave(arvore, raiz, chave);

    // Caso especial: raiz vazia
    if (raiz->numero_chaves == 0 && !raiz->eh_folha) {
        int nova_raiz_pos = raiz->filhos[0];
        No* nova_raiz = disk_read(arvore, nova_raiz_pos);
        nova_raiz->posicao_arq_binario = 0;
        nova_raiz->pai_offset = -1; // Nova raiz não tem pai
        disk_write(arvore, nova_raiz);
        
        // Atualiza a árvore
        arvore->numero_nos--; // Remove a raiz antiga
        liberaNo(raiz); // Libera a raiz antiga
        raiz = nova_raiz; // Atualiza referência
    }

    disk_write(arvore, raiz);
    liberaNo(raiz);
    return registro;
}


/*----------------DISK_READ--------------------------DISK_WRITE--------------------*/
No *disk_read(ArvoreB *arvore, int posicao)
{
    FILE *arquivo_bin = arvore->arq_binario;
    No *node = (No *)malloc(sizeof(No));
    fseek(arquivo_bin, posicao * arvore->tam_byte_node, SEEK_SET);

    fread(&node->numero_chaves, sizeof(int), 1, arquivo_bin);
    fread(&node->eh_folha, sizeof(bool), 1, arquivo_bin);
    fread(&node->lotado, sizeof(bool), 1, arquivo_bin);
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
    fwrite(&node->eh_folha, sizeof(bool), 1, arquivo_bin);
    fwrite(&node->lotado, sizeof(bool), 1, arquivo_bin);
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


/*------------------------ FUNÇÕES AUXILIARES ------------------------*/

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
static No* getPredecessor(ArvoreB *arvore, No *no) {
    No *atual = no;
    No *proximo = NULL;

    while (!atual->eh_folha) {
        proximo = disk_read(arvore, atual->filhos[atual->numero_chaves]);
        if (atual != no) liberaNo(atual); // Libera nós intermediários
        atual = proximo;
    }

    return atual;
}

static No* getSucessor(ArvoreB *arvore, No *no) {
    No *atual = no;
    No *proximo = NULL;

    while (!atual->eh_folha) {
        proximo = disk_read(arvore, atual->filhos[0]);
        if (atual != no) liberaNo(atual); // Libera nós intermediários
        atual = proximo;
    }

    return atual;
}

static int removeChave(ArvoreB *arvore, No *no, int chave) {
    int indice = percorreNo(no, chave);
    int registro = -1;

    // Caso 1: Chave encontrada
    if (indice < no->numero_chaves && no->chaves[indice] == chave) {
        registro = no->valores[indice];

        if (no->eh_folha) {
            removeDaFolha(no, indice);
            // Verifica underflow após remoção
            if (no->numero_chaves < MIN_CHAVES(arvore->ordem)) {
                tratarUnderflow(arvore, no, indice);
            }
        } else {
            removeDeNoInterno(arvore, no, indice);
        }
    }
    // Caso 2: Descer na árvore
    else {
        if (no->eh_folha) return -1;

        No *filho = disk_read(arvore, no->filhos[indice]);
        if (filho->numero_chaves <= MIN_CHAVES(arvore->ordem)) {
            tratarUnderflow(arvore, no, indice);
            // Re-lê o filho após reestruturação
            liberaNo(filho);
            filho = disk_read(arvore, no->filhos[indice]);
        }

        registro = removeChave(arvore, filho, chave);
        liberaNo(filho);
    }

    disk_write(arvore, no);
    return registro;
}

static void removeDaFolha(No *no, int indice) {
    for (int i = indice + 1; i < no->numero_chaves; i++) {
        no->chaves[i - 1] = no->chaves[i]; // Desloca chaves para a esquerda
        no->valores[i - 1] = no->valores[i]; // Desloca valores para a esquerda
    }
    no->numero_chaves--; // Atualiza o número de chaves
}

static void removeDeNoInterno(ArvoreB *arvore, No *no, int indice) {
    int chave = no->chaves[indice];
    No *filho_esq = disk_read(arvore, no->filhos[indice]);
    No *filho_dir = disk_read(arvore, no->filhos[indice + 1]);

    // Subcaso 2A: Filho esquerdo tem chaves suficientes
    if (filho_esq->numero_chaves > MIN_CHAVES(arvore->ordem)) {
        No* predecessor = getPredecessor(arvore, filho_esq);
        no->chaves[indice] = predecessor->chaves[predecessor->numero_chaves - 1];
        no->valores[indice] = predecessor->valores[predecessor->numero_chaves - 1];

        disk_write(arvore, no);
        removeChave(arvore, filho_esq, predecessor->chaves[predecessor->numero_chaves - 1]); // Remove a chave do filho esquerdo (até entao só havíamos copiado)
        
        liberaNo(predecessor);
    }
    // Subcaso 2B: Filho direito tem chaves suficientes
    else if (filho_dir->numero_chaves > MIN_CHAVES(arvore->ordem)) {
        No* sucessor = getSucessor(arvore, filho_dir);
        no->chaves[indice] = sucessor->chaves[0];
        no->valores[indice] = sucessor->valores[0];
        liberaNo(sucessor);

        disk_write(arvore, no);
        removeChave(arvore, filho_dir, sucessor->chaves[0]); // Remove a chave do filho direito (até entao só havíamos copiado)
    }
    // Subcaso 2C: Fusão necessária
    else {
        fundirNos(arvore, no, indice);
        removeChave(arvore, filho_esq, chave);
    }

    liberaNo(filho_esq);
    liberaNo(filho_dir);
}

static void fundirNos(ArvoreB *arvore, No *pai, int indice) {
    No *filho_esq = disk_read(arvore, pai->filhos[indice]);
    No *filho_dir = disk_read(arvore, pai->filhos[indice + 1]);

    //Move a chave do pai para o filho esquerdo
    filho_esq->chaves[filho_esq->numero_chaves] = pai->chaves[indice];
    filho_esq->valores[filho_esq->numero_chaves] = pai->valores[indice];
    filho_esq->numero_chaves++;

    //Copia chaves e valores do filho direito para o esquerdo
    for (int i = 0; i < filho_dir->numero_chaves; i++) {
        filho_esq->chaves[filho_esq->numero_chaves + i] = filho_dir->chaves[i];
        filho_esq->valores[filho_esq->numero_chaves + i] = filho_dir->valores[i];
    }

    //Copia ponteiros dos filhos (se não forem folhas)
    if (!filho_esq->eh_folha) {
        for (int i = 0; i <= filho_dir->numero_chaves; i++) {
            filho_esq->filhos[filho_esq->numero_chaves + i] = filho_dir->filhos[i];
        }
    }

    // Atualiza o número de chaves do filho esquerdo
    filho_esq->numero_chaves += filho_dir->numero_chaves;

    //Remove a chave do pai
    for (int i = indice; i < pai->numero_chaves - 1; i++) {
        pai->chaves[i] = pai->chaves[i + 1];
        pai->valores[i] = pai->valores[i + 1];
    }

    // Ajusta os ponteiros dos filhos do pai
    for (int i = indice + 1; i < pai->numero_chaves; i++) {
        pai->filhos[i] = pai->filhos[i + 1];
    }

    pai->numero_chaves--;

    // Passo 7: Atualiza o arquivo
    disk_write(arvore, pai);
    disk_write(arvore, filho_esq);

    // Passo 8: Libera memória
    liberaNo(filho_dir);
    arvore->numero_nos--; // O filho direito foi removido
}

static void tratarUnderflow(ArvoreB *arvore, No *pai, int indice) {
    No *filho = disk_read(arvore, pai->filhos[indice]);

    // Tenta pegar emprestado do irmão esquerdo
    if (indice > 0) {
        No *irmao_esq = disk_read(arvore, pai->filhos[indice - 1]);
        if (irmao_esq->numero_chaves > MIN_CHAVES(arvore->ordem)) {
            // Rotação direita
            for (int i = filho->numero_chaves; i > 0; i--) {
                filho->chaves[i] = filho->chaves[i - 1];
                filho->valores[i] = filho->valores[i - 1];
            }

            // Move a chave do pai para o filho
            filho->chaves[0] = pai->chaves[indice - 1];
            filho->valores[0] = pai->valores[indice - 1];
            filho->numero_chaves++;

            // Move a chave do irmão para o pai
            pai->chaves[indice - 1] = irmao_esq->chaves[irmao_esq->numero_chaves - 1];
            pai->valores[indice - 1] = irmao_esq->valores[irmao_esq->numero_chaves - 1];
            irmao_esq->numero_chaves--;

            disk_write(arvore, pai);
            disk_write(arvore, filho);
            disk_write(arvore, irmao_esq);

            liberaNo(irmao_esq);
            liberaNo(filho);
            return;
        }
        liberaNo(irmao_esq);
    }

    // Tenta pegar emprestado do irmão direito
    if (indice < pai->numero_chaves) {
        No *irmao_dir = disk_read(arvore, pai->filhos[indice + 1]);
        if (irmao_dir->numero_chaves > MIN_CHAVES(arvore->ordem)) {
            // Rotação esquerda
            filho->chaves[filho->numero_chaves] = pai->chaves[indice];
            filho->valores[filho->numero_chaves] = pai->valores[indice];
            filho->numero_chaves++;

            // Move a chave do irmão para o pai
            pai->chaves[indice] = irmao_dir->chaves[0];
            pai->valores[indice] = irmao_dir->valores[0];

            // Desloca as chaves do irmão
            for (int i = 0; i < irmao_dir->numero_chaves - 1; i++) {
                irmao_dir->chaves[i] = irmao_dir->chaves[i + 1];
                irmao_dir->valores[i] = irmao_dir->valores[i + 1];
            }

            irmao_dir->numero_chaves--;

            disk_write(arvore, pai);
            disk_write(arvore, filho);
            disk_write(arvore, irmao_dir);

            liberaNo(irmao_dir);
            liberaNo(filho);
            return;
        }
        liberaNo(irmao_dir);
    }

    // Se não conseguiu emprestar, funde com um irmão
    if (indice > 0) {
        fundirNos(arvore, pai, indice - 1); // Funde com o irmão esquerdo
    } else {
        fundirNos(arvore, pai, indice); // Funde com o irmão direito
    }

    liberaNo(filho);
}