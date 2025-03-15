#ifndef ARVORE_H
#define ARVORE_H

typedef struct __arvoreB ArvoreB;

ArvoreB *criaArvore(int ordem, int chave, int dado);

ArvoreB *criaArvoreVazia(int ordem);

ArvoreB *insereArvore(ArvoreB *raiz, int chave, int dado);

ArvoreB *retiraArvore(ArvoreB *raiz, int chave);

void buscaArvore(ArvoreB *raiz, int chave);

void printaResumo(ArvoreB *raiz);

#endif
#ifndef ARVORE_H
#define ARVORE_H

#include "no.h"

typedef struct arvore ArvoreB;

ArvoreB *criaArvore(int ordem);

ArvoreB *criaArvoreVazia(int ordem);

ArvoreB *insereArvore(ArvoreB *raiz, int chave, int dado);

tNo *retiraArvore(ArvoreB *raiz, int chave);

void buscaArvore(ArvoreB *raiz, int chave);

void printaResumo(ArvoreB *raiz);

#endif