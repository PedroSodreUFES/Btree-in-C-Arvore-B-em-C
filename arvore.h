#ifndef ARVORE_H
#define ARVORE_H

#include "no.h"

typedef struct arvore tArvore;

tArvore *criaArvore(int ordem);

tArvore *criaArvoreVazia(int ordem);

tArvore *insereArvore(tArvore *raiz, int chave, int dado);

tNo *retiraArvore(tArvore *raiz, int chave);

void buscaArvore(tArvore *raiz, int chave);

void printaResumo(tArvore *raiz);

#endif