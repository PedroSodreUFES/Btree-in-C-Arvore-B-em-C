#ifndef ARVORE_H
#define ARVORE_H

typedef struct arvore tArvore;

tArvore *criaArvore(int ordem, int chave, int dado);

tArvore *criaArvoreVazia(int ordem);

tArvore *insereArvore(tArvore *raiz, int chave, int dado);

tArvore *retiraArvore(tArvore *raiz, int chave);

void buscaArvore(tArvore *raiz, int chave);

void printaResumo(tArvore *raiz);

#endif