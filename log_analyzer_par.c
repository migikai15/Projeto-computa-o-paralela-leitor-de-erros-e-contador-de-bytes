#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    long long erros404;
    long long total_bytes;
} Stats;

typedef struct {
    char **linhas;
    int inicio;
    int fim;
    long long local404;
    long long localBytes;
} ThreadInfos;

Stats stats = {0, 0};
pthread_mutex_t mutex;

void* processar_linhas(void *arg) {
    ThreadInfos *data = (ThreadInfos*) arg;
    data->local404 = 0;
    data->localBytes = 0;

    for (int i = data->inicio; i < data->fim; i++) {
        char *linha = data->linhas[i];
        char *ptr = linha;
        
        //percorre a linha para achar 404 e adiciona 1 a local404
        while ((ptr = strstr(ptr, " 404")) != NULL) {
            data->local404++;
            ptr += 4;
        }

        ptr = linha;
        //percorre a linha para achar 200 e somar o valor apos 200 a localBytes 
        while ((ptr = strstr(ptr, " 200")) != NULL) {
            long long valor;
            if (sscanf(ptr + 4, "%lld", &valor) == 1) {
                data->localBytes += valor;
            }
            ptr += 4;
        }
    }

    //atualiza as estatisticas globais usando mutex
    //pthread_mutex_lock(&mutex);
    stats.erros404 += data->local404;
    stats.total_bytes += data->localBytes;
    //pthread_mutex_unlock(&mutex);
    printf("Thread ");
    return NULL;
}


int main(int argc, char *argv[]) {

    char *nomeArquivo = argv[1];

    //converte de string para int
    int numThreads = atoi(argv[2]);

    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo %s\n", nomeArquivo);
        return 1;
    }

    //le todas as linhas do arquivo para um array
    char **linhas = NULL;
    size_t capacidade = 0, numLinhas = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), arquivo)) {
        if (numLinhas >= capacidade) {
            capacidade = capacidade ? capacidade * 2 : 1024;
            linhas = realloc(linhas, capacidade * sizeof(char*));
        }
        linhas[numLinhas++] = strdup(buffer);
    }

    fclose(arquivo);

    pthread_t threads[numThreads];
    //cria um ThreadInfos para cada thread
    ThreadInfos threadData[numThreads];

    pthread_mutex_init(&mutex, NULL);

    //calcula quantas linhas cada thread ira ler
    int linhasPorThread = numLinhas / numThreads;
    int resto = numLinhas % numThreads;

    clock_t inicio = clock();

    //divide as linhas do arquivo entre as threads
    int inicioLinha = 0; 
    for (int i = 0; i < numThreads; i++) {
        int fimLinha = inicioLinha + linhasPorThread + (i < resto ? 1 : 0);
        threadData[i].linhas = linhas;
        threadData[i].inicio = inicioLinha;
        threadData[i].fim = fimLinha;
        pthread_create(&threads[i], NULL, processar_linhas, &threadData[i]);
        inicioLinha = fimLinha;
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    printf("Contagem de erros 404: %lld\n", stats.erros404);
    printf("Total de bytes transferidos: %lld\n", stats.total_bytes);
    printf("Tempo de execucao paralela com (%d)Threads: %.3f segundos\n", numThreads, tempo);

    for (size_t i = 0; i < numLinhas; i++)
        free(linhas[i]);
    free(linhas);
    pthread_mutex_destroy(&mutex);

    return 0;
}
