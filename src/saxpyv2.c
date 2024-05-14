#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>

extern int n_threads;
int n_threads = 2; // Initialize n_threads with a default value
#define NUM_THREADS 2

struct ThreadData {
    int id;
    int max_iters;
    int p;
    double a;
    double* X;
    double* Y;
    double* Y_avgs;
};

void* saxpy(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;

    int start = data->id * (data->p / n_threads);
    int end = (data->id + 1) * (data->p / n_threads);
    if (data->id == n_threads - 1) {
        // Ajuste para el último hilo
        end = data->p;
    }

    for (int it = 0; it < data->max_iters; it++) {
        for (int i = start; i < end; i++) {
            data->Y[i] = data->Y[i] + data->a * data->X[i];
            data->Y_avgs[it] += data->Y[i];
        }
        data->Y_avgs[it] = data->Y_avgs[it] / data->p;
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    // Variables to obtain command line parameters
    unsigned int seed = 1;
    int p = 10000000;
    int n_threads = 2;
    int max_iters = 1000;
    // Variables to perform SAXPY operation
    double* X;
    double a;
    double* Y;
    double* Y_avgs;

    // Getting input values
    int opt;
    while ((opt = getopt(argc, argv, ":p:s:n:i:")) != -1) {
        switch (opt) {
            case 'p':
                printf("vector size: %s\n", optarg);
                p = strtol(optarg, NULL, 10);
                assert(p > 0 && p <= 2147483647);
                break;
            case 's':
                printf("seed: %s\n", optarg);
                seed = strtol(optarg, NULL, 10);
                break;
            case 'n':
                printf("threads number: %s\n", optarg);
                n_threads = strtol(optarg, NULL, 10);
                break;
            case 'i':
                printf("max. iterations: %s\n", optarg);
                max_iters = strtol(optarg, NULL, 10);
                break;
            case ':':
                printf("option -%c needs a value\n", optopt);
                break;
            case '?':
                fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>] [-i <maximum itertions>]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    srand(seed);

    printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", p, seed, n_threads, max_iters);

    // initializing data
    X = (double*)malloc(sizeof(double) * p);
    Y = (double*)malloc(sizeof(double) * p);
    Y_avgs = (double*)malloc(sizeof(double) * max_iters);

    for (int i = 0; i < p; i++) {
        X[i] = (double)rand() / RAND_MAX;
        Y[i] = (double)rand() / RAND_MAX;
    }
    for (int i = 0; i < max_iters; i++) {
        Y_avgs[i] = 0.0;
    }
    a = (double)rand() / RAND_MAX;

#ifdef DEBUG
    printf("vector X= [ ");
    for (int i = 0; i < p - 1; i++) {
        printf("%f, ", X[i]);
    }
    printf("%f ]\n", X[p - 1]);

    printf("vector Y= [ ");
    for (int i = 0; i < p - 1; i++) {
        printf("%f, ", Y[i]);
    }
    printf("%f ]\n", Y[p - 1]);

    printf("a= %f \n", a);
#endif

    // Crear datos de los hilos
    pthread_t threads[n_threads];
    struct ThreadData thread_data[n_threads];
    for (int t = 0; t < n_threads; t++) {
        thread_data[t].id = t;
        thread_data[t].max_iters = max_iters;
        thread_data[t].p = p;
        thread_data[t].a = a;
        thread_data[t].X = X;
        thread_data[t].Y = Y;
        thread_data[t].Y_avgs = Y_avgs;
    }

    // Iniciar los hilos
    struct timeval t_start, t_end;
    gettimeofday(&t_start, NULL);

    for (int t = 0; t < n_threads; t++) {
        pthread_create(&threads[t], NULL, saxpy, (void*)&thread_data[t]);
    }

    // Después de unir los hilos en la función main
    for (int t = 0; t < n_threads; t++) {
        pthread_join(threads[t], NULL);
    }

    // Calcular Y_avgs después de que todos los hilos hayan terminado
    for (int it = 0; it < max_iters; it++) {
        for (int i = 0; i < p; i++) {
            Y_avgs[it] += Y[i];
        }
        Y_avgs[it] = Y_avgs[it] / p;
    }
    
    gettimeofday(&t_end, NULL);
    // End threading

#ifdef DEBUG
    printf("RES: final vector Y= [ ");
    for (int i = 0; i < p - 1; i++) {
        printf("%f, ", Y[i]);
    }
    printf("%f ]\n", Y[p - 1]);
#endif

    // Compute execution time
    double exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
    exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms

    printf("Execution time: %f ms \n", exec_time);
    printf("Last 3 values of Y: %f, %f, %f \n", Y[p - 3], Y[p - 2], Y[p - 1]);
    printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters - 3], Y_avgs[max_iters - 2], Y_avgs[max_iters - 1]);

    // Free allocated memory
    free(X);
    free(Y);
    free(Y_avgs);

    return 0;
}
