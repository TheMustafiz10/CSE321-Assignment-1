#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int n;
    int* fibonacci_sequence;
} fib_data_t;

typedef struct {
    int* fibonacci_sequence;
    int sequence_length;
    int search_index;
    int result;
} search_data_t;

void* compute_fibonacci(void* arg);
void* search_fibonacci(void* arg);

int main() {
    int n, search_count;
    pthread_t fib_thread, search_thread;
    fib_data_t fib_data;
    search_data_t* search_data_array = NULL;
    
    printf("Enter the term of fibonacci sequence:\n");
    scanf("%d", &n);

    if (n < 0 || n > 40) {
        printf("Error: n must be between 0 and 40\n");
        return 1;
    }
    
    printf("How many numbers you are willing to search?:\n");
    scanf("%d", &search_count);
    
    if (search_count <= 0) {
        printf("Error: number of searches must be greater than 0\n");
        return 1;
    }

    fib_data.n = n;
    fib_data.fibonacci_sequence = NULL;
    if (pthread_create(&fib_thread, NULL, compute_fibonacci, &fib_data) != 0) {
        printf("Error creating Fibonacci thread\n");
        return 1;
    }
    
    if (pthread_join(fib_thread, NULL) != 0) {
        printf("Error joining Fibonacci thread\n");
        return 1;
    }
    
    printf("Fibonacci sequence:\n");
    for (int i = 0; i <= n; i++) {
        printf("a[%d] = %d\n", i, fib_data.fibonacci_sequence[i]);
    }
    
    search_data_array = (search_data_t*)malloc(search_count * sizeof(search_data_t));
    if (search_data_array == NULL) {
        printf("Error allocating memory for search data\n");
        free(fib_data.fibonacci_sequence);
        return 1;
    }
    
    for (int i = 0; i < search_count; i++) {
        printf("Enter search %d:\n", i + 1);
        scanf("%d", &search_data_array[i].search_index);
        
        search_data_array[i].fibonacci_sequence = fib_data.fibonacci_sequence;
        search_data_array[i].sequence_length = n + 1;
        search_data_array[i].result = -1;
    }
    
    pthread_t* search_threads = (pthread_t*)malloc(search_count * sizeof(pthread_t));
    if (search_threads == NULL) {
        printf("Error allocating memory for search threads\n");
        free(fib_data.fibonacci_sequence);
        free(search_data_array);
        return 1;
    }
    
    for (int i = 0; i < search_count; i++) {
        if (pthread_create(&search_threads[i], NULL, search_fibonacci, &search_data_array[i]) != 0) {
            printf("Error creating search thread %d\n", i + 1);
        }
    }
    
    for (int i = 0; i < search_count; i++) {
        if (pthread_join(search_threads[i], NULL) != 0) {
            printf("Error joining search thread %d\n", i + 1);
        }
    }
    
    for (int i = 0; i < search_count; i++) {
        printf("result of search #%d = %d\n", i + 1, search_data_array[i].result);
    }
    
    free(fib_data.fibonacci_sequence);
    free(search_data_array);
    free(search_threads);
    
    return 0;
}



void* compute_fibonacci(void* arg) {
    fib_data_t* data = (fib_data_t*)arg;
    int n = data->n;
    data->fibonacci_sequence = (int*)malloc((n + 1) * sizeof(int));
    if (data->fibonacci_sequence == NULL) {
        printf("Error allocating memory for Fibonacci sequence\n");
        return NULL;
    }

    if (n >= 0) {
        data->fibonacci_sequence[0] = 0;
    }
    if (n >= 1) {
        data->fibonacci_sequence[1] = 1;
    }
    
    for (int i = 2; i <= n; i++) {
        data->fibonacci_sequence[i] = data->fibonacci_sequence[i-1] + data->fibonacci_sequence[i-2];
    }
    
    return NULL;
}


void* search_fibonacci(void* arg) {
    search_data_t* data = (search_data_t*)arg;
    
    if (data->search_index >= 0 && data->search_index < data->sequence_length) {
        data->result = data->fibonacci_sequence[data->search_index];
    } else {
        data->result = -1;
    }
    
    return NULL;
}