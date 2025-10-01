#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

int main(int argc, char *argv[]) {
    if (argc != 2) { 
        printf("Uso: %s nome_do_arquivo.txt\n", argv[0]);
        return 1;
    }

    FILE *arquivo;
    char linha[1024];
    int quant404 = 0;
    long long soma200 = 0; 

    clock_t inicio = clock();

    arquivo = fopen(argv[1], "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo %s!\n", argv[1]);
        return 1;
    }

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        char *ptr = linha;
        
        //percore a linha e caso ache 404 adiciona 1 a count404
        while ((ptr = strstr(ptr, " 404")) != NULL) {
            quant404++;
            ptr += 4; 
        }
        //percore a linha e caso ache 200 soma o valor seguinte a soma200
        ptr = linha; 
        while ((ptr = strstr(ptr, " 200")) != NULL) {
            long long valor;
            if (sscanf(ptr + 4, "%lld", &valor) == 1) { 
                soma200 += valor;
            }
            ptr += 4; 
        }
    }

    fclose(arquivo);

    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    printf("\nContagem de erros 404: %d\n", quant404);
    printf("Total de bytes transferidos: %lld\n", soma200);
    printf("Tempo de execução sequencial: %.3f segundos\n", tempo);

    return 0;
}
