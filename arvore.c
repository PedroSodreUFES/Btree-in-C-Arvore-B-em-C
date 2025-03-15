#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct ChaveRegistro {
    int chave;   // Valor da chave
    int registro; // Valor do conteúdo associado à chave
} ChaveRegistro;

typedef struct No {
    ChaveRegistro* chaves_registro;            // Array de chaves armazenadas no nó (deve conter até ordem - 1 chaves)
    struct No **filhos;       // Ponteiros para os filhos
    int numero_chaves;     // Número atual de chaves no nó
    struct No *pai;           // Ponteiro para o nó pai
    int eh_folha;             // 1 = folha. 0 = não folha (facilita as operações)
} No;

struct __arvoreB{
    No *raiz;
    int ordem; // Ordem da árvore B (número máximo de filhos)
};

ArvoreB *criaArvoreB(int ordem){
    ArvoreB *aB = malloc(sizeof(ArvoreB));
    aB->ordem = ordem;
    aB->raiz = NULL;
    return aB;
}


/*
    @brief Insere na Árvore um novo nó. OBS: Em caso de chave repetida, não insere.

    @param raiz Nó raiz.
    @param chave Chave do dado.
    @param dado Dado contido naquela chave.

    @return A nova árvore balanceada.
*/
ArvoreB *insereArvore(ArvoreB *raiz, int chave, int dado){
    return NULL;
}

/*
    @brief Retira da Árvore um nó encontrado.

    @param raiz Nó raiz.
    @param chave Chave do dado buscado.

    @return O nó procurado ou NULL em caso da inexistência do nó na árvore.
*/
ArvoreB *retiraArvore(ArvoreB *raiz, int chave){
    return NULL;
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