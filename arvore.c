#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct BTreeNode {
    int *chaves, chave, numero_de_filhos, ordem;
    struct BTreeNode **filhos, *pai;
} BTreeNode;

struct __bTree{
    BTreeNode *raiz;
    int ordem; // Ordem da árvore B (número máximo de filhos)
};

BTree *criaArvoreVazia(int ordem){
    return NULL;
}

/*
    @brief Aloca um novo nó.

    @param ordem Ordem da árvore B.
    @param chave Chave do valor procurado.
    @param dado Dado contido naquele nó.

    @return A arvore a alocada.
*/
BTree *criaArvore(int ordem, int chave, int dado){
    BTree *a = malloc(1*sizeof(BTree));
    a->ordem = ordem;
    a->numero_de_filhos = 0;
    a->chave = chave;
    a->chaves = NULL;
    a->filhos = NULL;
    a->pai = NULL;
    return a;
}

/*
    @brief Insere na Árvore um novo nó. OBS: Em caso de chave repetida, não insere.

    @param raiz Nó raiz.
    @param chave Chave do dado.
    @param dado Dado contido naquela chave.

    @return A nova árvore balanceada.
*/
BTree *insereArvore(BTree *raiz, int chave, int dado){
    return NULL;
}

/*
    @brief Retira da Árvore um nó encontrado.

    @param raiz Nó raiz.
    @param chave Chave do dado buscado.

    @return O nó procurado ou NULL em caso da inexistência do nó na árvore.
*/
BTree *retiraArvore(BTree *raiz, int chave){
    return NULL;
}

/*
    @brief Imprime na tela se o nó está contido na Árvore B ou não.
    Em caso de presença, imprime: O REGISTRO ESTA NA ARVORE!
    Caso contrário, imprime: O REGISTRO NAO ESTA NA ARVORE! 
    @param raiz Nó raiz.
    @param chave Chave do dado buscado.
*/
void buscaArvore(BTree *raiz, int chave){

}

/*
    @brief Percorre a árvore e printa o estado final de maneira balanceada.

    @param raiz O nó raiz
*/
void printaResumo(BTree *raiz){

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