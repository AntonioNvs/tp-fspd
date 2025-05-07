/* 

Trabalho Prático 1 - Fundamentos de Sistemas Paralelos e Distribuídos

Nome: Antônio Caetano Neves Neto
Matrícula: 2022043698

O seguinte código é a solução completa do TP, no qual contém todas as funções detalhadas
e especificadas seu funcionamento. O código foi baseado em conter uma matriz de registro de
qual grupo está ocupando a posição agora e qual a quantidade de elementos presente no momento
naquela posição, sendo cada posição na matriz uma variável de condição que o elemento pode esperar,
caso deseja entrar e não consiga.

Os comandos a seguir é somente para salvamento de uso próprio.

./ex1 < cases/4.txt | sort -n > cases/my_output4.txt
diff -w cases/my_output4.txt cases/s4.txt

*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "passa_tempo.h"

#define MAXN 25

int N, n_threads;

// Estrutura a informação de posição dada na entrada.
typedef struct position {
    int x;
    int y;
    int time;
} position_t;

// Estrutura os atributos da thread que é passado na entrada.
typedef struct thread {
    int identifier;
    int group;
    int n_positions;
    position_t* positions;
} thread_attr_t;

// Estrutura como o grid será formado.
typedef struct grid {
    int group;
    int quantity;
} grid_t;

// Define as variáveis globais que envolvem o controle da seção crítica, que no caso
// é o acesso a matriz grid.
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conditions[MAXN][MAXN];
grid_t grid[MAXN][MAXN];

// Responsável por inicializar as variáveis de condição, o grid e o mutex
void init() {
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++) {
            pthread_cond_init(&conditions[i][j], NULL);
            grid[i][j].group = -1;
            grid[i][j].quantity = 0;
        }

    pthread_mutex_init(&lock, NULL);
}

// Realiza o processo de saída do sistema de controle da seção crítica.
void destroy() {
    for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        pthread_cond_destroy(&conditions[i][j]);
    
    pthread_mutex_destroy(&lock);
}

void leave_position(int crr_pos, thread_attr_t* t) {
    int prev_x = t->positions[crr_pos-1].x;
    int prev_y = t->positions[crr_pos-1].y;

    grid[prev_x][prev_y].quantity--;

    // Se tiver só ele na posição, então ela vai ser ocupada por mais ninguém, fazendo assim um signal
    // para que uma thread de outro grupo possa entrar.
    if(grid[prev_x][prev_y].quantity == 0) {
        grid[prev_x][prev_y].group = -1;
        pthread_cond_signal(&conditions[prev_x][prev_y]);
    } else {
        // Caso contrário, é feito um broadcast para que todas as threads do mesmo grupo possam entrar
        pthread_cond_broadcast(&conditions[prev_x][prev_y]);
    }   
}

// Função principal, onde dita o fluxo de cada thread.
void* execute(void* arg) {
    thread_attr_t* t = (thread_attr_t*) arg;

    int crr_pos = 0;

    // Enquanto não passou por todas as posições, continua executando.
    while(crr_pos < t->n_positions) {
        int x = t->positions[crr_pos].x;
        int y = t->positions[crr_pos].y;

        // Será entrado na seção crítica, ou seja, a variável grid será utilizada.
        pthread_mutex_lock(&lock);
        
        // Espera até que seja possível acessar a posição desejada.
        // Para tal, a posição deve estar sendo ocupada por um ou mais elementos do mesmo grupo que a thread
        // ou não ter nenhum elemento ocupando ela.
        while((grid[x][y].group != -1) && (grid[x][y].group != t->group)) {
            pthread_cond_wait(&conditions[x][y], &lock);
        }

        // Se não haver nenhum grupo ocupando a thread, atualiza o valor correspondente.
        if(grid[x][y].group == -1)
            grid[x][y].group = t->group;

        grid[x][y].quantity++;

        // Verifica se é a primeira posição que está sendo executada, se não, faz o processo
        // de saída da posição que o elemento estava, já que ele pode ir para a próxima.
        if(crr_pos > 0) {
            leave_position(crr_pos, t);
        }

        // Sai da seção crítica, pois grid não será mais utilizado.
        pthread_mutex_unlock(&lock);

        // Utiliza da função passa_tempo
        passa_tempo(t->identifier, x, y, t->positions[crr_pos].time);

        // Passa para a próxima posição
        crr_pos++;

        // Caso não haja mais posições, o elemento sai do grid.
        if(crr_pos == t->n_positions) {
            pthread_mutex_lock(&lock);
            leave_position(crr_pos, t);
            pthread_mutex_unlock(&lock);
        }
    }

    return NULL;
}

int main() {
    // Leitura dos atributos iniciais
    scanf("%d %d", &N, &n_threads);
    
    thread_attr_t threads_attributes[n_threads];

    // Leitura dos atributos das threads
    for(int i = 0; i < n_threads; i++) {
        scanf(
            "%d %d %d", 
            &threads_attributes[i].identifier, 
            &threads_attributes[i].group, 
            &threads_attributes[i].n_positions
        );
        int n_positions = threads_attributes[i].n_positions;

        // Aloca espaço para positions, dado que ela é criada como um ponteiro, precisando de alocação dinâmica.
        threads_attributes[i].positions = malloc(n_positions * sizeof(position_t));

        // Salva as informações das sequências e posições dada na entrada.
        for (int j = 0; j < n_positions; j++) { 
            scanf(
                "%d %d %d", 
                &threads_attributes[i].positions[j].x,
                &threads_attributes[i].positions[j].y,
                &threads_attributes[i].positions[j].time
            );
        }
    }

    init();
    pthread_t threads_executions[n_threads];

    // Cria e começa a executar todas as threads, passando seus respectivos atributos.
    for(int i = 0; i < n_threads; i++)
        pthread_create(&threads_executions[i], NULL, execute, &threads_attributes[i]);

    // Espera todas as threads terminarem sua execução
    for(int i = 0; i < n_threads; i++)
        pthread_join(threads_executions[i], NULL);

    destroy();

    return 0;
}