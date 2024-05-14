#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <pthread.h>

typedef struct {
    int thread_id;
    int n_threads;
    int p;
    int max_iters;
    double a;
    double* X;
    double* Y;
    double* Y_part_sum;
} thread_data_t;

void* saxpy_thread(void* arg) {
    thread_data_t* data = (thread_data_t*) arg;
    int thread_id = data->thread_id;
    int n_threads = data->n_threads;
    int p = data->p;
    int max_iters = data->max_iters;
    double a = data->a;
    double* X = data->X;
    double* Y = data->Y;
    double* Y_part_sum = data->Y_part_sum;

    int start = thread_id * p / n_threads;
    int end = (thread_id + 1) * p / n_threads;
    for (int it = 0; it < max_iters; it++) {
        double partial_sum = 0.0;
        for (int i = start; i < end; i++) {
            Y[i] = Y[i] + a * X[i];
            partial_sum += Y[i];
        }
        Y_part_sum[thread_id * max_iters + it] = partial_sum;
    }
    return NULL;
}

int main(int argc, char* argv[]){
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
    int i, it;
    // Variables to get execution time
    struct timeval t_start, t_end;
    double exec_time;

    // Getting input values
    int opt;
    while ((opt = getopt(argc, argv, ":p:s:n:i:")) != -1) {  
        switch(opt) {  
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
                fprintf(stderr, "Usage: %s [-p <vector size>] [-s <seed>] [-n <threads number>] [-i <maximum iterations>]\n", argv[0]);
                exit(EXIT_FAILURE);
        }  
    }  
    srand(seed);

    printf("p = %d, seed = %d, n_threads = %d, max_iters = %d\n", p, seed, n_threads, max_iters);    

    // initializing data
    X = (double*) malloc(sizeof(double) * p);
    Y = (double*) malloc(sizeof(double) * p);
    Y_avgs = (double*) malloc(sizeof(double) * max_iters);

    for (i = 0; i < p; i++) {
        X[i] = (double)rand() / RAND_MAX;
        Y[i] = (double)rand() / RAND_MAX;
    }
    for (i = 0; i < max_iters; i++) {
        Y_avgs[i] = 0.0;
    }
    a = (double)rand() / RAND_MAX;

#ifdef DEBUG
    printf("vector X= [ ");
    for (i = 0; i < p-1; i++) {
        printf("%f, ", X[i]);
    }
    printf("%f ]\n", X[p-1]);

    printf("vector Y= [ ");
    for (i = 0; i < p-1; i++) {
        printf("%f, ", Y[i]);
    }
    printf("%f ]\n", Y[p-1]);

    printf("vector Y_avgs= [ ");
	for(int i = 0; i < max_iters-1; i++){
		printf("%f, ", Y_avgs[i]);
	}
	printf("%f ]\n", Y_avgs[max_iters-1]);

    printf("a= %f \n", a);    
#endif

    pthread_t* threads = malloc(n_threads * sizeof(pthread_t));
    thread_data_t* thread_data = malloc(n_threads * sizeof(thread_data_t));
    double* Y_part_sum = malloc(sizeof(double) * max_iters * n_threads);

    // Initialize thread data. Experimenting with posix_memalign
    for (int i = 0; i < n_threads; i++) {
        posix_memalign((void**)&thread_data[i].Y_part_sum, 64, max_iters * sizeof(double));
        thread_data[i].thread_id = i;
        thread_data[i].n_threads = n_threads;
        thread_data[i].p = p;
        thread_data[i].max_iters = max_iters;
        thread_data[i].a = a;
        thread_data[i].X = X;
        thread_data[i].Y = Y;
    }

    gettimeofday(&t_start, NULL);

    // Create threads
    for (i = 0; i < n_threads; i++) {
        pthread_create(&threads[i], NULL, saxpy_thread, &thread_data[i]);
    }

    // Wait for all threads to finish
    for (i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    gettimeofday(&t_end, NULL);

    // Compute final Y_avgs by summing partial sums from each thread
    for (it = 0; it < max_iters; it++) {
        for (i = 0; i < n_threads; i++) {
            Y_avgs[it] += Y_part_sum[i * max_iters + it];
        }
        Y_avgs[it] = Y_avgs[it] / p;
    }

#ifdef DEBUG
    printf("RES: final vector Y= [ ");
    for (i = 0; i < p-1; i++) {
        printf("%f, ", Y[i]);
    }
    printf("%f ]\n", Y[p-1]);

    printf("Final vector Y_avgs= [ ");
	for(int i = 0; i < max_iters-1; i++){
		printf("%f, ", Y_avgs[i]);
	}
	printf("%f ]\n", Y_avgs[max_iters-1]);
#endif
    
    // Computing execution time
    exec_time = (t_end.tv_sec - t_start.tv_sec) * 1000.0;  // sec to ms
    exec_time += (t_end.tv_usec - t_start.tv_usec) / 1000.0; // us to ms
    printf("Execution time: %f ms \n", exec_time);
    printf("Last 3 values of Y: %f, %f, %f \n", Y[p-3], Y[p-2], Y[p-1]);
    printf("Last 3 values of Y_avgs: %f, %f, %f \n", Y_avgs[max_iters-3], Y_avgs[max_iters-2], Y_avgs[max_iters-1]);

    free(X);
    free(Y);
    free(Y_avgs);
    free(Y_part_sum);

    return 0;
    
}
