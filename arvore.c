#include "arvore.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MIN_CHAVES(ordem) ((ordem)/2 - 1)
#define MAX_CHAVES(ordem) (ordem - 1)

/*------------------------ STRUCTS ------------------------*/
typedef struct ChaveRegistro {
    int chave;   // Valor da chave
    int registro; // Valor do conteúdo associado à chave
} ChaveRegistro;

typedef struct No {
    ChaveRegistro* chaves_registro;  // Array de chaves armazenadas no nó (deve conter até ordem - 1 chaves)
    int numero_chaves;  // Número atual de chaves no nó
    char eh_folha;  // Facilita as operações
    char lotado;  // Se estiver lotado e for inserido mais uma chave, é necessário dividir
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

/*------------------------ FUNÇÕES DISCO ------------------------*/

// Cria e lê um nó do arquivo binário a partir de sua posição
No* disk_read(ArvoreB* arvore, int posicao) {
    FILE* arquivo_bin = arvore->arq_binario;
    No* node = (No*)malloc(sizeof(No));
    if (!node) return NULL;

    fseek(arquivo_bin, posicao * arvore->tam_byte_node, SEEK_SET);

    // Lê numero_chaves, eh_folha (como int), posicao_arq_binario
    fread(&node->numero_chaves, sizeof(int), 1, arquivo_bin);
    fread(&node->eh_folha, sizeof(char), 1, arquivo_bin);
    fread(&node->posicao_arq_binario, sizeof(int), 1, arquivo_bin);

    int ordem = arvore->ordem;
    node->chaves_registro = (ChaveRegistro*)malloc(sizeof(ChaveRegistro) * (ordem - 1));
    node->filhos = (int*)malloc(sizeof(int) * ordem);

    fread(node->chaves_registro, sizeof(ChaveRegistro), ordem - 1, arquivo_bin);
    fread(node->filhos, sizeof(int), ordem, arquivo_bin);

    node->lotado = node->numero_chaves == MAX_CHAVES(ordem) ? '1' : '0';
    return node;
}

// Escreve um nó no arquivo binário a partir de sua posição e libera a memória do nó
void disk_write(ArvoreB* arvore, No* node) {
    FILE* arquivo_bin = arvore->arq_binario;
    fseek(arquivo_bin, node->posicao_arq_binario * arvore->tam_byte_node, SEEK_SET);

    fwrite(&node->numero_chaves, sizeof(int), 1, arquivo_bin);
    fwrite(&node->eh_folha, sizeof(char), 1, arquivo_bin);
    fwrite(&node->posicao_arq_binario, sizeof(int), 1, arquivo_bin);
    fwrite(node->chaves_registro, sizeof(ChaveRegistro), arvore->ordem - 1, arquivo_bin);
    fwrite(node->filhos, sizeof(int), arvore->ordem, arquivo_bin);
    fflush(arquivo_bin); // Força a escrita no arquivo para ocorrer imediatamente (não quando o sistema decidir -> possivelmente nao precisa)
    liberaNo(node);
}

/*------------------------ FUNÇÕES NÓ ------------------------*/

// Não cria no binário (talvez tenha que criar)
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

// Retorna um nó a partir de sua posição no arquivo binário (NAO SEI SE TA CERTO)
static No* retornaNo(ArvoreB *arvore, int posicao){
    No *no = (No*)malloc(sizeof(No));
    fseek(arvore->arq_binario, posicao * arvore->tam_byte_node, SEEK_SET); // SEEK_SET: referência = início do arquivo
    fread(no, arvore->tam_byte_node, 1, arvore->arq_binario);
    return no;
}

// Insere a chave no nó de forma ordenada (mantendo a ordem decrescente) e atualiza o número de chaves
static void insereChaveNo (No *node, ChaveRegistro chave) {
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

// Retorna a posição da chave a ser inserida.
static int percorreNo (No* no, int chave) {
    int i = 0;
    while (i < no->numero_chaves && chave > no->chaves_registro[i].chave) i++;
    return i; // Para uma posição antes da chave maior que a chave a ser inserida
}

void insereNoBinario(ArvoreB *arvore, No *no){
    fseek(arvore->arq_binario, no->posicao_arq_binario * arvore->tam_byte_node, SEEK_SET);
    fwrite(no, arvore->tam_byte_node, 1, arvore->arq_binario);
}

void liberaNo (No *no) {
    free(no->chaves_registro);
    free(no->filhos);
    free(no);
}

/*------------------------------------------------*/


/*------------------------ FUNÇÕES ÁRVORE ------------------------*/

// Inicializa a árvore B com tudo nulo
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


    if (aB->numero_nos == 0) { // Árvore vazia
        No *raiz = criaNo(aB->ordem);
        insereChaveNo(raiz, chave_registro);
        aB->numero_nos++;
        aB->arq_binario = fopen("arvore.bin", "wb"); // Só fecha quando o programa termina
        insereNoBinario(aB, raiz);
        return;
    }

    // No* no_atual = aB->raiz;

    // while (!no_atual->eh_folha) { // Inserido apenas em nós folhas
    //     int i = percorreNo (no_atual, chave);

    //     No *filho = no_atual->filhos[i];
    //     if (filho->numero_chaves == aB->ordem - 1) { // Limite de chaves de um nó atingido
    //         divideNo (aB, filho);

    //         if (chave_registro.chave > no_atual->chaves_registro[i].chave) i++; // Como uma nova chave foi inserida, é necessário verificar atualizar o índice i
    //         filho = no_atual->filhos[i];
    //     }
    //     no_atual = filho;
    // }

    // insereNo (no_atual, chave_registro);
    // if (no_atual->numero_chaves == aB->ordem) { // Limite de ordem - 1 chaves atingido
    //     divideNo (aB, no_atual);
    // }
}

/*------------------------ FUNÇÕES REMOÇÃO ------------------------*/
/*
    @brief Retira da Árvore um nó encontrado.

    @param arvore Árvore B.
    @param chave Chave do dado buscado.

    @return O conteúdo encontrado pela chave a ser removida da árvore.
*/
int retiraArvore(ArvoreB *arvore, int chave){
}

int buscaArvore(ArvoreB *arvore, int chave){
    No* no_atual = retornaNo(arvore, 0); // Raiz
    int i;

    while (true) { // Possível loop infinito (nao testado)
        i = percorreNo(no_atual, chave); // Intervalo que a chave pode estar

        if (no_atual->chaves_registro[i].chave == chave) // Chave encontrada
            return no_atual->chaves_registro[i].registro;

        if (no_atual->eh_folha) // Se for folha, não tem mais o que procurar
            return -1;

        no_atual = retornaNo(arvore, no_atual->filhos[i]);
    }

    return -1; // Chave não encontrada
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