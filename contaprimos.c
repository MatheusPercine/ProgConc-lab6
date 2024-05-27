#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>

int *buffer; // buffer compartilhado
int M; // tamanho M do buffer
int N; // numero N da sequência de inteiros
int nthreads; // numero de threads consumidoras

sem_t vazio; // semaforo para controlar o espaço vazio no buffer
sem_t cheio; // semaforo para controlar o espaço cheio no buffer
pthread_mutex_t mutex; // mutex para proteger o acesso ao buffer

int in = 0; // indice de insercao no buffer
int out = 0; // indice de remocao do buffer
int *file_data; // dados lidos do arquivo
int file_index = 0; // indice de leitura dos dados do arquivo

long long int contador = 0; // contador de numeros primos
long long int *contador_thread; // contador de primos por thread

// funcao que verifica se eh primo
int ehPrimo(long long int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    for (int i = 3; i <= sqrt(n); i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

// funcao executada pela thread produtora
void *produtora(void *arg) {
    // abre arquivo e le seus dados
    FILE *file = fopen((char *)arg, "rb");
    if (!file) {
        fprintf(stderr, "Erro de abertura do arquivo de entrada\n");
        pthread_exit(NULL);
    }

    fread(&N, sizeof(int), 1, file);
    file_data = (int *)malloc(N * sizeof(int));
    if (!file_data) {
        fprintf(stderr, "Erro de alocação de memória para os dados do arquivo\n");
        fclose(file);
        pthread_exit(NULL);
    }

    fread(file_data, sizeof(int), N, file);
    fclose(file);

    for (int i = 0; i < N; i++) {
        sem_wait(&vazio); // Espera por espaço vazio no buffer
        pthread_mutex_lock(&mutex); // Trava o buffer

        buffer[in] = file_data[i];
        in = (in + 1) % M;

        pthread_mutex_unlock(&mutex); // Destrava o buffer
        sem_post(&cheio); // Sinaliza que há um item cheio no buffer
    }

    // Após a produção, sinalize o fim para as threads consumidoras
    for (int i = 0; i < nthreads; i++) {
        sem_wait(&vazio); // Espera por espaço vazio no buffer
        pthread_mutex_lock(&mutex); // Trava o buffer

        buffer[in] = -1; // -1 como sinal de término
        in = (in + 1) % M;

        pthread_mutex_unlock(&mutex); // Destrava o buffer
        sem_post(&cheio); // Sinaliza que há um item cheio no buffer
    }

    free(file_data);
    pthread_exit(NULL);
}

// funcao executada pelas threads consumidoras
void *consumidora(void *arg) {
    int id = *(int *)arg;
    int item;
    while (1) {
        sem_wait(&cheio); // Espera por um item cheio no buffer
        pthread_mutex_lock(&mutex); // Trava o buffer

        item = buffer[out];
        out = (out + 1) % M;

        pthread_mutex_unlock(&mutex); // Destrava o buffer
        sem_post(&vazio); // Sinaliza que há um espaço vazio no buffer

        if (item == -1) {
            break; // Termina a thread quando encontra o sinal de término
        }

        if (ehPrimo(item)) {
            pthread_mutex_lock(&mutex); // Trava o contador
            contador++;
            contador_thread[id]++;
            pthread_mutex_unlock(&mutex); // Destrava o contador
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t tprod; //thread produtora
    pthread_t *tcon; //threads consumidoras
    int *id; //id das threads consumidoras

    if (argc < 4) {
        fprintf(stderr, "Digite: %s <numero de threads consumidoras> <tamanho do buffer> <arquivo de entrada>\n", argv[0]);
        return 1;
    }

    nthreads = atoi(argv[1]);
    M = atoi(argv[2]);

    // alocacoes de memoria para o "buffer" e o "contador de primos por thread"
    buffer = (int *)malloc(M * sizeof(int));
    if (buffer == NULL) {
        fprintf(stderr, "Erro de alocação de memória para o buffer\n");
        return 2;
    }

    contador_thread = (long long int *)calloc(nthreads, sizeof(long long int));
    if (contador_thread == NULL) {
        fprintf(stderr, "Erro de alocação de memória para os contadores das threads\n");
        free(buffer);
        return 3;
    }


    //aloca memoria para as threads
    tcon = (pthread_t *)malloc(nthreads * sizeof(pthread_t));
    id = (int *)malloc(nthreads * sizeof(int));
    if (tcon == NULL || id == NULL) {
        fprintf(stderr, "Erro de alocação de memória para as threads consumidoras\n");
        free(buffer);
        free(contador_thread);
        if (tcon) free(tcon);
        if (id) free(id);
        return 4;
    }
    

    // inicia semaforo e mutex
    sem_init(&vazio, 0, M);
    sem_init(&cheio, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // cria as threads
    if(pthread_create(&tprod, NULL, produtora, (void *)argv[3])){
        printf("--ERRO: pthread_create()\n"); exit(-1);
    };

    for (int i = 0; i < nthreads; i++) {
        id[i] = i;
        if (pthread_create(&tcon[i], NULL, consumidora, &id[i])) {
            printf("--ERRO: pthread_create()\n"); exit(-1);
        }
    }

    //espera threads terminarem
    pthread_join(tprod, NULL);
    for (int i = 0; i < nthreads; i++) {
        pthread_join(tcon[i], NULL);
    }

    //mostra os resultados
    int vencedora = 0;
    for (int i = 1; i < nthreads; i++) {
        if (contador_thread[i] > contador_thread[vencedora]) {
            vencedora = i;
        }
    }
    printf("Quantidade de numeros primos encontrados: %lld\n", contador);
    printf("Thread vencedora eh %d com %lld primos\n", vencedora, contador_thread[vencedora]);

    //libera memoria
    sem_destroy(&vazio);
    sem_destroy(&cheio);
    pthread_mutex_destroy(&mutex);
    free(buffer);
    free(tcon);
    free(id);
    free(contador_thread);

    return 0;
}
