#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#define MAXN 25

int N, n_threads;

typedef struct position {
    int x;
    int y;
    int time;
} position_t;

typedef struct thread {
    int identifier;
    int group;
    int n_positions;
    position_t* positions;

} thread_attr_t;

typedef struct grid {
    int group;
    int quantity;
} grid_t;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t conditions[MAXN][MAXN];
grid_t grid[MAXN][MAXN];

void init() {
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++) {
            pthread_cond_destroy(&conditions[i][j]);
            grid[i][j].group = -1;
            grid[i][j].quantity = 0;
        }
}

void destroy() {
    for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
        pthread_cond_destroy(&conditions[i][j]);
}

void passa_tempo(int tid, int x, int y, int decimos) {
    struct timespec zzz, agora;
    static struct timespec inicio = {0,0};
    int tstamp;

    if ((inicio.tv_sec == 0)&&(inicio.tv_nsec == 0)) {
        clock_gettime(CLOCK_REALTIME,&inicio);
    }

    zzz.tv_sec  = decimos/10;
    zzz.tv_nsec = (decimos%10) * 100L * 1000000L;

    clock_gettime(CLOCK_REALTIME,&agora);
    tstamp = ( 10 * agora.tv_sec  +  agora.tv_nsec / 100000000L )
            -( 10 * inicio.tv_sec + inicio.tv_nsec / 100000000L );

    printf("%3d [ %2d @(%2d,%2d) z%4d\n",tstamp,tid,x,y,decimos);

    nanosleep(&zzz,NULL);

    clock_gettime(CLOCK_REALTIME,&agora);
    tstamp = ( 10 * agora.tv_sec  +  agora.tv_nsec / 100000000L )
            -( 10 * inicio.tv_sec + inicio.tv_nsec / 100000000L );

    printf("%3d ) %2d @(%2d,%2d)\n",tstamp,tid,x,y);
}

void* execute(void* arg) {
    thread_attr_t* t = (thread_attr_t*) arg;

    int crr_pos = 0;

    while(crr_pos < t->n_positions) {
        int x = t->positions[crr_pos].x;
        int y = t->positions[crr_pos].y;

        pthread_mutex_lock(&lock);
        
        // printf("%d %d %d %d\n", grid[x][y].group, x, y, (grid[x][y].group != -1) && (grid[x][y].group != t->group));

        while((grid[x][y].group != -1) && (grid[x][y].group != t->group)) {
            pthread_cond_wait(&conditions[x][y], &lock);
        }

        if(grid[x][y].group == -1)
            grid[x][y].group = t->group;

        grid[x][y].quantity++;

        if(crr_pos > 0) {
            int prev_x = t->positions[crr_pos-1].x;
            int prev_y = t->positions[crr_pos-1].y;

            grid[prev_x][prev_y].quantity--;

            // printf("%d\n", grid[prev_x][prev_y].quantity);

            if(grid[prev_x][prev_y].quantity == 0) {
                grid[prev_x][prev_y].group = -1;
                pthread_cond_signal(&conditions[prev_x][prev_y]);
            } else {
                pthread_cond_broadcast(&conditions[prev_x][prev_y]);
            }            
        }

        pthread_mutex_unlock(&lock);
        passa_tempo(t->identifier, x, y, t->positions[crr_pos].time);

        crr_pos++;
    }

    return NULL;
}

int main() {
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

        threads_attributes[i].positions = malloc(n_positions * sizeof(position_t));

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

    for(int i = 0; i < n_threads; i++)
        pthread_create(&threads_executions[i], NULL, execute, &threads_attributes[i]);

    for(int i = 0; i < n_threads; i++)
        pthread_join(threads_executions[i], NULL);

    destroy();
    pthread_mutex_destroy(&lock);

    return 0;
}