#ifndef ARVORE_H
#define ARVORE_H

typedef struct __arvoreB ArvoreB;

ArvoreB *criaArvore(int ordem, int chave, int dado);

ArvoreB *criaArvoreVazia(int ordem);

ArvoreB *insereArvore(ArvoreB *arvore, int chave, int dado);

ArvoreB *retiraArvore(ArvoreB *arvore, int chave);

int buscaArvore(ArvoreB *arvore, int chave); //Retorna o conteúdo da chave (se não existir, retorna -1??) nao sei se terao registros negativos

void printaResumo(ArvoreB *arvore);

#endif