#ifndef ARVORE_H
#define ARVORE_H

#include <stdio.h>

typedef struct __arvoreB ArvoreB;

typedef struct no No;

/* funcoes da arvore*/
ArvoreB *criaArvoreB(int ordem, FILE *binario);
void criaArvoreManual(ArvoreB *arvore);
void imprimeArvore(ArvoreB *arvore, int posicao, int nivel);      
ArvoreB *insereArvore(ArvoreB *arvore, int chave, int dado);
ArvoreB *divideArvore(int offset, ArvoreB *sentinela);
int retiraArvore(ArvoreB *arvore, int chave); // PDF do professor não imprime nada na remoção, então é possível que possa ser void
int buscaArvore(ArvoreB *arvore, int chave); //Retorna o conteúdo da chave (se não existir, retorna -1??) nao sei se terao registros negativos
void printaResumo(ArvoreB *arvore);

/* FUNÇÕES DE NÓ*/
No *criaNo(ArvoreB *arv);
int getOffset(No *no);
char getLotado(No *no);
void liberaNo(No *no);
void printaChaves(No *no);

/* FUNÇÕES DISK*/
void disk_write(ArvoreB *arvore, No *node);
No *disk_read(ArvoreB *arvore, int posicao);

#endif