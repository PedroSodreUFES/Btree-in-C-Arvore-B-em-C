#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*------------------------ STRUCTS ------------------------*/
typedef struct ChaveRegistro {
    int chave;   // Valor da chave
    int registro; // Valor do conteúdo associado à chave
} ChaveRegistro;

typedef struct No {
    ChaveRegistro* chaves_registro;  // Array de chaves armazenadas no nó (deve conter até ordem - 1 chaves)
    int numero_chaves;  // Número atual de chaves no nó
    bool eh_folha;  // Facilita as operações
    bool lotado;  // Se estiver lotado e for inserido mais uma chave, é necessário dividir
    int posicao_arq_binario;  // Índice do nó no arquivo binário (posição será calculada com índice*tam_byte_node)
    int* filhos;  // Array de índices dos filhos no arquivo binário
} No;

struct __arvoreB{
    //Raiz será detectada pelo índice 0 do arquivo
    int ordem; // Ordem da árvore B (número máximo de filhos)
    int numero_nos; // Número de nós na árvore
    int tam_byte_node; // Tamanho em bytes de um nó
    FILE* arq_binario; // Arquivo binário que armazena a árvore
};
/*------------------------------------------------*/

/*------------------------ FUNÇÕES NÓ ------------------------*/

static No* criaNo(int ordem){
    No *novo_no = (No*)malloc(sizeof(No));
    novo_no->chaves_registro = (ChaveRegistro*)malloc((ordem - 1) * sizeof(ChaveRegistro));
    novo_no->filhos = (int*) malloc(ordem * sizeof(int));
    novo_no->numero_chaves = 0;
    novo_no->eh_folha = 1; // Começa como folha e vai subindo
    for (int i = 0; i < ordem; i++) {
        novo_no->filhos[i] = -1; // Inicializa com -1
    }
    return novo_no;
}

// Insere a chave no nó de forma ordenada (mantendo a ordem decrescente) e atualiza o número de chaves
static void insereNo (No *node, ChaveRegistro chave) {
    int i = node->numero_chaves - 1;
    while (i >= 0 && chave.chave < node->chaves_registro[i].chave) {
        node->chaves_registro[i + 1] = node->chaves_registro[i];
        i--;
    }
    int pos = i + 1;
    node->chaves_registro[pos] = chave;
    node->numero_chaves++;
}

static void divideNo (ArvoreB* arvore, No* no) {
}

// Retorna a posição da chave a ser inserida
static int percorreNo (No* no, int chave) {
    int i = 0;
    while (i < no->numero_chaves && chave > no->chaves_registro[i].chave) i++;
    return i; // Para uma posição antes da chave maior que a chave a ser inserida
}

/*------------------------------------------------*/


/*------------------------ FUNÇÕES ÁRVORE ------------------------*/

ArvoreB *criaArvoreB(int ordem){
    ArvoreB *aB = malloc(sizeof(ArvoreB));
    aB->ordem = ordem;
    aB->tam_byte_node = sizeof(No);
    aB->numero_nos = 0;
    aB->arq_binario = NULL;
    return aB;
}

/*
    @brief Insere na Árvore um novo nó. OBS: Em caso de chave repetida, não insere.

    @param raiz Nó raiz.
    @param chave Chave do dado.
    @param dado Dado contido naquela chave.

    @return A nova árvore balanceada.
*/
void insereArvore(ArvoreB *aB, int chave, int dado){
    ChaveRegistro chave_registro;
    chave_registro.chave = chave;
    chave_registro.registro = dado;

    if (aB->raiz == NULL){
        aB->raiz = criaNo(aB->ordem);
        aB->raiz->chaves_registro[0] = chave_registro;
        aB->raiz->numero_chaves = 1;
        //aB->arq_binario = ???
        return;
    }

    No* no_atual = aB->raiz;

    while (!no_atual->eh_folha) { // Inserido apenas em nós folhas
        int i = percorreNo (no_atual, chave);

        No *filho = no_atual->filhos[i];
        if (filho->numero_chaves == aB->ordem - 1) { // Limite de chaves de um nó atingido
            divideNo (aB, filho);

            if (chave_registro.chave > no_atual->chaves_registro[i].chave) i++; // Como uma nova chave foi inserida, é necessário verificar atualizar o índice i
            filho = no_atual->filhos[i];
        }
        no_atual = filho;
    }

    insereNo (no_atual, chave_registro);
    if (no_atual->numero_chaves == aB->ordem) { // Limite de ordem - 1 chaves atingido
        divideNo (aB, no_atual);
    }
}

/*
    @brief Retira da Árvore um nó encontrado.

    @param arvore Árvore B.
    @param chave Chave do dado buscado.

    @return O conteúdo encontrado pela chave a ser removida da árvore.
*/
int retiraArvore(ArvoreB *arvore, int chave){

}

int buscaArvore(ArvoreB *arvore, int chave){
    No *no_atual = arvore->raiz;
    while (no_atual) { // Se torna NULL para os filhos de uma folha
        int i = percorreNo(no_atual, chave);
        if (i < no_atual->numero_chaves && no_atual->chaves_registro[i].chave == chave)
            return no_atual->chaves_registro[i].registro; // Encontrou
        // if (no_atual->eh_folha) 
        //     break;
        no_atual = no_atual->filhos[i];
    }
    return -1; // Não encontrou
}

/*
    @brief Imprime na tela se o nó está contido na Árvore B ou não.
    Em caso de presença, imprime: O REGISTRO ESTA NA ARVORE!
    Caso contrário, imprime: O REGISTRO NAO ESTA NA ARVORE! 
    @param raiz Nó raiz.
    @param chave Chave do dado buscado.
*/
void buscaArvore(ArvoreB *raiz, int chave){

}

/*
    @brief Percorre a árvore e printa o estado final de maneira balanceada.

    @param raiz O nó raiz
*/
void printaResumo(ArvoreB *raiz){

}


/* -------------------------------------------------------------------------
    NOTAS                                                                  |
----------------------------------------------------------------------------
    raiz = folha ou no minimo 2 filhos.                                    |
    cada no(sem ser folha ou raiz) possui no MÍNIMO (ordem + 1) filhos.    |
    cada no tem no máximo (2*ordem + 1) filhos.                            |
    TODAS as folhas estão no mesmo nível.                                  |
                                                                           |
    Página: nome dado a um nó de uma árvore B                              |
        |                                                                  |
        |--> Cada página armazenad diversos registros da tabela original   |
----------------------------------------------------------------------------
*/