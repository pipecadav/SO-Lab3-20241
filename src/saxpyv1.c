/**
 * @defgroup   SAXPY saxpy
 *
 * @brief      This file implements an iterative saxpy operation
 * 
 * @param[in] <-p> {vector size} 
 * @param[in] <-s> {seed}
 * @param[in] <-n> {number of threads to create} 
 * @param[in] <-i> {maximum itertions} 
 *
 * @author     Danny Munera
 * @date       2020
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>

#define NUM_THREADS 3

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

    int start = (data->id == 0) ? 0 : (data->p / 2);
    int end = (data->id == 0) ? (data->p / 2) : data->p;

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

    printf("vector Y_avgs= [ ");
	for(int i = 0; i < max_iters-1; i++){
		printf("%f, ", Y_avgs[i]);
	}
	printf("%f ]\n", Y_avgs[max_iters-1]);

    printf("a= %f \n", a);
#endif

    // Create thread data
    pthread_t threads[NUM_THREADS];
    struct ThreadData thread_data[NUM_THREADS];
    for (int t = 0; t < NUM_THREADS; t++) {
        thread_data[t].id = t;
        thread_data[t].max_iters = max_iters;
        thread_data[t].p = p;
        thread_data[t].a = a;
        thread_data[t].X = X;
        thread_data[t].Y = Y;
        thread_data[t].Y_avgs = Y_avgs;
    }

    // Start threads
    struct timeval t_start, t_end;
    gettimeofday(&t_start, NULL);

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_create(&threads[t], NULL, saxpy, (void*)&thread_data[t]);
    }

    // Join threads
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    gettimeofday(&t_end, NULL);
    // End threading

#ifdef DEBUG
    printf("RES: final vector Y= [ ");
    for (int i = 0; i < p - 1; i++) {
        printf("%f, ", Y[i]);
    }
    printf("%f ]\n", Y[p - 1]);
    
	printf("Final vector Y_avgs= [ ");
	for(int i = 0; i < max_iters-1; i++){
		printf("%f, ", Y_avgs[i]);
	}
	printf("%f ]\n", Y_avgs[max_iters-1]);
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
