/* Programa que cria arquivo com um vetor de valores do tipo int, gerados aleatoriamente 
 * Entrada: tamanho do vetor e nome do arquivo de saida
 * Saida: arquivo binario com a dimensão (valores inteiros) do vetor, 
 * seguido dos valores (int) de todas as celulas do vetor gerados aleatoriamente
 * */

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <math.h>

// Descomentar o define abaixo caso deseje imprimir uma versão truncada do vetor gerado no formato texto
// #define TEXTO 

long long int contador = 0;

// Verifica, de forma sequencial, quantos primos existem na sequência de N inteiros gerada
int ehPrimo(long long int n) {
    int i;
    if (n<=1) return 0;
    if (n==2) return 1;
    if (n%2==0) return 0;
    for (i=3; i<sqrt(n)+1; i+=2)
    if(n%i==0) return 0;
    return 1;
}

int main(int argc, char* argv[]) {
    int *buffer; // vetor de inteiros que será gerado
    int N; // tamanho do vetor
    FILE *descritorArquivo; // descritor do arquivo de saída
    size_t ret; // retorno da função de escrita no arquivo de saída

    // recebe os argumentos de entrada
    if (argc < 3) {
        fprintf(stderr, "Digite: %s <N> <arquivo saida>\n", argv[0]);
        return 1;
    }

    N = atoi(argv[1]); 

    // aloca memória para o vetor
    buffer = (int*) malloc(sizeof(int) * N);
    if (!buffer) {
        fprintf(stderr, "Erro de alocacao da memoria do buffer\n");
        return 2;
    }

    // preenche o vetor com valores inteiros aleatórios
    // randomiza a sequência de números aleatórios
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        buffer[i] = rand() % 1000;
    }

    // imprimir na saída padrão o vetor gerado
    #ifdef TEXTO
    for (int i = 0; i < N; i++) {
        fprintf(stdout, "%d ", buffer[i]);
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n");
    #endif

    long long int resultado;
    for (long long int i = 1; i<=N; i++){
        resultado = ehPrimo(buffer[i]);
        if (resultado == 1){
            contador++;
        }
        #ifdef TESTE
        if (resultado == 1){
            printf("%lld e primo\n", i);
        }
        #endif
    }

    printf("quantidade de primos: %lld \n", contador);

    // escreve o vetor no arquivo
    // abre o arquivo para escrita binária
    descritorArquivo = fopen(argv[2], "wb");
    if (!descritorArquivo) {
        fprintf(stderr, "Erro de abertura do arquivo\n");
        return 3;
    }
    // escreve o tamanho do vetor
    ret = fwrite(&N, sizeof(int), 1, descritorArquivo);
    if (ret < 1) {
        fprintf(stderr, "Erro de escrita no arquivo\n");
        fclose(descritorArquivo);
        return 4;
    }
    // escreve os elementos do vetor
    ret = fwrite(buffer, sizeof(int), N, descritorArquivo);
    if (ret < N) {
        fprintf(stderr, "Erro de escrita no arquivo\n");
        fclose(descritorArquivo);
        return 4;
    }

    // finaliza o uso das variáveis
    fclose(descritorArquivo);
    free(buffer);
    return 0;
}