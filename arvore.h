#ifndef ARVORE_H
#define ARVORE_H

#include <stdio.h>

typedef struct __arvoreB ArvoreB;

typedef struct no No;

/* funcoes da arvore*/
ArvoreB *criaArvoreB(int ordem, FILE *binario);
ArvoreB *insereArvore(ArvoreB *sentinela, int chave, int valor);
ArvoreB *divideArvore(int offset, ArvoreB *sentinela);
int retiraChave(ArvoreB *arvore, int chave); // PDF do professor não imprime nada na remoção, então é possível que possa ser void
int retornaOffsetRaiz(ArvoreB *arv);
void buscaNo(ArvoreB *arv, int chave);

/* FUNÇÕES DE NÓ*/
No *criaNo(ArvoreB *arv);
No *criaNoVazio(ArvoreB *arv);
int getOffset(No *no);
char getLotado(No *no);
void liberaNo(No *no);
void printaChaves(No *no);
void printaValores(No *no);
char retornaFolha(No *no);
int *retornaFilhos(No *no);
int retornaNumChaves(No *no);
int *retornaChaves(No *no);

/* FUNÇÕES DISK*/
void disk_write(ArvoreB *arvore, No *node);
No *disk_read(ArvoreB *arvore, int posicao);
void printaFilhos(No *no);
void teste();

#endif