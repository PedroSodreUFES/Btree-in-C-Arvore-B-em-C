#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "arvore.h"

 /*
        -----------------------------
        ETAPA DE LEITURA DE ARQUIVOS;
        1- recebe numero de operações
        2- recebe a ordem da árvore
        3- recebe em um loop a
            sequência de operações
        -----------------------------
*/

int main(int argc, char **argv)
{
    /*
    -----------------------
        LEITURA DE ARQUIVOS
    -----------------------
    */
    if (argc < 3)
    {
        printf("É preciso fornecer um arquivo de entrada e outro de saída para rodar o programa!\nRode o programa no formato ./trab2 <entrada.txt> <saida.txt>\n");
        exit(1);
    }
    FILE *input, *output;

    char *input_path = argv[1];
    char *output_path = argv[2];

    input = fopen(input_path, "r");
    if (input == NULL)
    {
        printf("Arquivo de leitura '%s' não pode ser aberto corretamente!", input_path);
        exit(1);
    }

    output = fopen(output_path, "w");
    if (output == NULL)
    {
        printf("Arquivo de escrita '%s' não pode ser aberto corretamente!", output_path);
        exit(1);
    }

    // Pega a ordem da arvore e o numero de comandos no arquivo txt
    int numero_de_comandos, ordem_da_arvore;
    fscanf(input, "%d%*[^0-9.]", &ordem_da_arvore);
    fscanf(input, "%d%*[^IBR]", &numero_de_comandos);

    tArvore *primeiro = criaArvoreVazia(ordem_da_arvore);

    // processa os comandos
    char comando;
    int chave, dado;
    tArvore *aux;
    for(int i=0 ; i<numero_de_comandos ; i++)
    {
        fscanf(input, "%c%*c", &comando);
        if(comando == 'I')
        {
            fscanf(input,"%d, %d%*[^RBI]", &chave, &dado);
            primeiro = insereArvore(primeiro, chave, dado);
        }
        else if(comando == 'R')
        {
            fscanf(input,"%d%*[^RBI]", &chave);
            aux = retiraArvore(primeiro, chave);
        }
        else if(comando == 'B')
        {
            fscanf(input, "%d%*[^RBI]", &chave);
            buscaArvore(primeiro, chave);
        }
        else{
            printf("Comando inválido detectado!\nO comando %c não tem uma definicao!\n", comando);
            fscanf(input, "%*[^IRB]");
        }
    }

    fclose(input);
    fclose(output);
    return 0;
}