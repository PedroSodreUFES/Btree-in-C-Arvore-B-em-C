#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <unistd.h>
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


/*------------------------ FUNÇÕES AUXILIARES ------------------------*/
static int percorreNo (No* no, int chave);
static int removeChave(ArvoreB* arvore, No* no, int chave);
static void removeDaFolha(No* no, int indice);
static void removeDeNoInterno(ArvoreB* arvore, No* no, int indice);
static void fundirNos(ArvoreB *arvore, No *pai, int indice);
static void tratarUnderflow(ArvoreB* arvore, No* pai, int indice);
static bool emprestaDoIrmaoEsquerdo(ArvoreB* arvore, No* pai, int indice);
static bool emprestaDoIrmaoDireito(ArvoreB* arvore, No* pai, int indice);
static No* getPredecessor(ArvoreB *arvore, No *no);
static No* getSucessor(ArvoreB *arvore, No *no);
/*--------------------Nó-----------------------*/

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

void criaArvoreManual(ArvoreB *arvore) {
    // Configurações
    int ordem = 3;
    arvore->ordem = ordem;
    arvore->numero_nos = 12; // Total de nós criados
    arvore->tam_byte_node = sizeof(No);

    // ------------------------
    // Nível 0: Raiz
    // ------------------------
    No *raiz = criaNo(arvore);
    raiz->posicao_arq_binario = 0;
    raiz->eh_folha = 0;
    raiz->chaves[0] = 30;   // Valores da raiz
    raiz->chaves[1] = 60;
    raiz->valores[0] = 300; // Conteúdos associados
    raiz->valores[1] = 600;
    raiz->numero_chaves = 2;
    raiz->filhos[0] = 1;    // Filho 1 (posição 1)
    raiz->filhos[1] = 2;    // Filho 2 (posição 2)
    raiz->filhos[2] = 3;    // Filho 3 (posição 3)
    disk_write(arvore, raiz);

    // ------------------------
    // Nível 1: Filhos da Raiz
    // ------------------------

    // Filho 1 (posição 1)
    No *filho1 = criaNo(arvore);
    filho1->posicao_arq_binario = 1;
    filho1->eh_folha = 0;
    filho1->chaves[0] = 10;
    filho1->chaves[1] = 20;
    filho1->valores[0] = 100;
    filho1->valores[1] = 200;
    filho1->numero_chaves = 2;
    filho1->filhos[0] = 4;  // Neto 1 (posição 4)
    filho1->filhos[1] = 5;  // Neto 2 (posição 5)
    filho1->filhos[2] = 6;  // Neto 3 (posição 6)
    filho1->pai_offset = 0; // Pai é a raiz (posição 0)
    disk_write(arvore, filho1);

    // Filho 2 (posição 2)
    No *filho2 = criaNo(arvore);
    filho2->posicao_arq_binario = 2;
    filho2->eh_folha = 0;
    filho2->chaves[0] = 40;
    filho2->chaves[1] = 50;
    filho2->valores[0] = 400;
    filho2->valores[1] = 500;
    filho2->numero_chaves = 2;
    filho2->filhos[0] = 7;  // Neto 4 (posição 7)
    filho2->filhos[1] = 8;  // Neto 5 (posição 8)
    filho2->filhos[2] = 9;  // Neto 6 (posição 9)
    filho2->pai_offset = 0; // Pai é a raiz (posição 0)
    disk_write(arvore, filho2);

    // Filho 3 (posição 3)
    No *filho3 = criaNo(arvore);
    filho3->posicao_arq_binario = 3;
    filho3->eh_folha = 0;
    filho3->chaves[0] = 70;
    filho3->valores[0] = 700;
    filho3->numero_chaves = 1;
    filho3->filhos[0] = 10; // Neto 7 (posição 10)
    filho3->filhos[1] = 11; // Neto 8 (posição 11)
    filho3->pai_offset = 0; // Pai é a raiz (posição 0)
    disk_write(arvore, filho3);

    // ------------------------
    // Nível 2: Netos (Folhas)
    // ------------------------

    // Neto 1 (posição 4)
    No *neto1 = criaNo(arvore);
    neto1->posicao_arq_binario = 4;
    neto1->eh_folha = 1;
    neto1->chaves[0] = 5;
    neto1->valores[0] = 50;
    neto1->chaves[1] = 10;
    neto1->valores[1] = 100;
    neto1->numero_chaves = 2;
    neto1->pai_offset = 1; // Pai é o filho1 (posição 1)
    disk_write(arvore, neto1);

    // Neto 2 (posição 5)
    No *neto2 = criaNo(arvore);
    neto2->posicao_arq_binario = 5;
    neto2->eh_folha = 1;
    neto2->chaves[0] = 15;
    neto2->valores[0] = 150;
    neto2->numero_chaves = 1;
    neto2->pai_offset = 1;
    disk_write(arvore, neto2);

    // Neto 3 (posição 6)
    No *neto3 = criaNo(arvore);
    neto3->posicao_arq_binario = 6;
    neto3->eh_folha = 1;
    neto3->chaves[0] = 25;
    neto3->valores[0] = 250;
    neto3->numero_chaves = 1;
    neto3->pai_offset = 1;
    disk_write(arvore, neto3);

    // Neto 4 (posição 7)
    No *neto4 = criaNo(arvore);
    neto4->posicao_arq_binario = 7;
    neto4->eh_folha = 1;
    neto4->chaves[0] = 35;
    neto4->valores[0] = 350;
    neto4->numero_chaves = 1;
    neto4->pai_offset = 2; // Pai é o filho2 (posição 2)
    disk_write(arvore, neto4);

    // Neto 5 (posição 8)
    No *neto5 = criaNo(arvore);
    neto5->posicao_arq_binario = 8;
    neto5->eh_folha = 1;
    neto5->chaves[0] = 45;
    neto5->valores[0] = 450;
    neto5->numero_chaves = 1;
    neto5->pai_offset = 2;
    disk_write(arvore, neto5);

    // Neto 6 (posição 9)
    No *neto6 = criaNo(arvore);
    neto6->posicao_arq_binario = 9;
    neto6->eh_folha = 1;
    neto6->chaves[0] = 55;
    neto6->valores[0] = 550;
    neto6->numero_chaves = 1;
    neto6->pai_offset = 2;
    disk_write(arvore, neto6);

    // Neto 7 (posição 10)
    No *neto7 = criaNo(arvore);
    neto7->posicao_arq_binario = 10;
    neto7->eh_folha = 1;
    neto7->chaves[0] = 65;
    neto7->valores[0] = 650;
    neto7->numero_chaves = 1;
    neto7->pai_offset = 3; // Pai é o filho3 (posição 3)
    disk_write(arvore, neto7);

    // Neto 8 (posição 11)
    No *neto8 = criaNo(arvore);
    neto8->posicao_arq_binario = 11;
    neto8->eh_folha = 1;
    neto8->chaves[0] = 75;
    neto8->valores[0] = 750;
    neto8->numero_chaves = 1;
    neto8->pai_offset = 3;
    disk_write(arvore, neto8);

    // Libera todos os nós da memória
    liberaNo(raiz);
    liberaNo(filho1);
    liberaNo(filho2);
    liberaNo(filho3);
    liberaNo(neto1);
    liberaNo(neto2);
    liberaNo(neto3);
    liberaNo(neto4);
    liberaNo(neto5);
    liberaNo(neto6);
    liberaNo(neto7);
    liberaNo(neto8);
}

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

int buscaArvore(ArvoreB *arvore, int chave) {
    No* raiz = disk_read(arvore, arvore->offsetRaiz);
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
        filhoMenor->lotado = '0';
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
        filhoMaior->lotado = '0';
        filhoMaior->pai_offset = 0;

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
        filhoMenor->lotado = '0';
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
        filhoMaior->lotado = '0';
        filhoMaior->pai_offset = antiga_raiz->posicao_arq_binario;

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
        arvore->numero_nos--; 
        liberaNo(raiz);
        liberaNo(nova_raiz);
        return registro;
    } 
        
    disk_write(arvore, raiz); // Atualiza a raiz se não foi substituída
    liberaNo(raiz);
    return registro;
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
    fwrite(&node->eh_folha, sizeof(bool), 1, arquivo_bin);
    fwrite(&node->lotado, sizeof(bool), 1, arquivo_bin);
    fwrite(&node->posicao_arq_binario, sizeof(int), 1, arquivo_bin);
    fwrite(&node->pai_offset, sizeof(int), 1, arquivo_bin);

    fwrite(node->chaves, sizeof(int), ordem, arquivo_bin);
    fwrite(node->valores, sizeof(int), ordem, arquivo_bin);
    fwrite(node->filhos, sizeof(int), ordem + 1, arquivo_bin);
    fflush(arquivo_bin);
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


/*------------------------ FUNÇÕES AUXILIARES ------------------------*/

static int percorreNo (No* no, int chave) {
    int i = 0;
    while (chave > no->chaves[i] && i < no->numero_chaves) i++;
    return i;
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