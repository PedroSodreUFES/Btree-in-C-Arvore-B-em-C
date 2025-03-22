#ifndef ARVORE_H
#define ARVORE_H

#include <stdio.h>

typedef struct arvoreB ArvoreB;

/* Cria uma nova árvore B com a ordem especificada */
ArvoreB* criaArvoreB(int ordem, FILE* arquivoBinario);

/* Insere uma chave e valor na árvore. Retorna a árvore atualizada */
ArvoreB* insereArvore(ArvoreB* arvore, int chave, int valor);

/* Remove uma chave da árvore. Retorna o valor associado ou -1 se não encontrado */
ArvoreB* retiraArvore(ArvoreB* arvore, int chave);

/* Busca uma chave na árvore. Retorna o valor associado ou -1 se não encontrado */
int buscaArvore(ArvoreB* arvore, int chave);

/* Imprime a estrutura da árvore para debug */
void imprimeArvore(ArvoreB* arvore, int posicaoRaiz, int nivel);

/* Retorna o offset da raiz no arquivo binário */
int retornaOffsetRaiz(ArvoreB* arvore);

/* Libera a memória da estrutura da árvore (não fecha o arquivo) */
void liberaArvore(ArvoreB* arvore);

#endif