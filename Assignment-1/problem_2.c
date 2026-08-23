#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>



typedef enum {
    BREAD,
    CHEESE,
    LETTUCE,
    INGREDIENT_COUNT
} Ingredient;



pthread_mutex_t table_mutex;
sem_t supplier_sem;
sem_t maker_sems[INGREDIENT_COUNT];
int table_ingredients[2];
int table_empty = 1;
int N;
int completed_cycles = 0;


void* supplier(void* arg);
void* sandwich_maker(void* arg);
const char* ingredient_name(Ingredient ing);
char maker_id(Ingredient missing_ing);
void place_ingredients();
Ingredient get_missing_ingredient();
void sort_ingredients();

int main() {
    printf("Enter the number of times supplier places ingredients: ");
    scanf("%d", &N);
    
    if (N <= 0) {
        printf("Error: N must be greater than 0\n");
        return 1;
    }
    

    pthread_mutex_init(&table_mutex, NULL);
    sem_init(&supplier_sem, 0, 1); 
    
    for (int i = 0; i < INGREDIENT_COUNT; i++) {
        sem_init(&maker_sems[i], 0, 0);
    }

    srand(time(NULL));
    
    pthread_t supplier_thread;
    pthread_t maker_threads[INGREDIENT_COUNT];

    pthread_create(&supplier_thread, NULL, supplier, NULL);
    


    Ingredient maker_ingredients[INGREDIENT_COUNT] = {BREAD, CHEESE, LETTUCE};
    for (int i = 0; i < INGREDIENT_COUNT; i++) {
        Ingredient* ing = malloc(sizeof(Ingredient));
        *ing = maker_ingredients[i];
        pthread_create(&maker_threads[i], NULL, sandwich_maker, ing);
    }
    
    pthread_join(supplier_thread, NULL);
    
    sleep(1);
    for (int i = 0; i < INGREDIENT_COUNT; i++) {
        pthread_cancel(maker_threads[i]);
    }
    

    for (int i = 0; i < INGREDIENT_COUNT; i++) {
        pthread_join(maker_threads[i], NULL);
    }

    pthread_mutex_destroy(&table_mutex);
    sem_destroy(&supplier_sem);
    for (int i = 0; i < INGREDIENT_COUNT; i++) {
        sem_destroy(&maker_sems[i]);
    }
    
    return 0;
}

void* supplier(void* arg) {
    for (int i = 0; i < N; i++) {
        sem_wait(&supplier_sem);
        
        pthread_mutex_lock(&table_mutex);

        place_ingredients();
        sort_ingredients();
        
        printf("Supplier places: %s and %s\n", 
               ingredient_name(table_ingredients[0]),
               ingredient_name(table_ingredients[1]));
        
        table_empty = 0;

        Ingredient missing_ing = get_missing_ingredient();
        sem_post(&maker_sems[missing_ing]);
        
        pthread_mutex_unlock(&table_mutex);
    }
    return NULL;
}

void* sandwich_maker(void* arg) {
    Ingredient my_ingredient = *(Ingredient*)arg;
    free(arg);
    
    while (1) {
        sem_wait(&maker_sems[my_ingredient]);
        
        pthread_mutex_lock(&table_mutex);
        
        if (!table_empty) {
            Ingredient current_missing = get_missing_ingredient();
            if (current_missing == my_ingredient) {
                sort_ingredients();


                printf("Maker %c picks up %s and %s\n", 
                       maker_id(my_ingredient),
                       ingredient_name(table_ingredients[0]),
                       ingredient_name(table_ingredients[1]));
                
                printf("Maker %c is making the sandwich...\n", maker_id(my_ingredient));


                usleep(500000);
                
                printf("Maker %c finished making the sandwich and eats it\n", maker_id(my_ingredient));
                printf("Maker %c signals Supplier\n", maker_id(my_ingredient));
                
                completed_cycles++;
                if (completed_cycles < N) {
                    printf("\n");
                }


                table_empty = 1;
                table_ingredients[0] = -1;
                table_ingredients[1] = -1;
                
                sem_post(&supplier_sem);
            }
        }
        
        pthread_mutex_unlock(&table_mutex);
        


        if (completed_cycles >= N) {
            break;
        }
    }
    return NULL;
}



void place_ingredients() {
    int ing1, ing2;
    
    do {
        ing1 = rand() % INGREDIENT_COUNT;
        ing2 = rand() % INGREDIENT_COUNT;
    } while (ing1 == ing2); 
    
    table_ingredients[0] = ing1;
    table_ingredients[1] = ing2;
}



void sort_ingredients() {
    if (table_ingredients[0] > table_ingredients[1]) {
        int temp = table_ingredients[0];
        table_ingredients[0] = table_ingredients[1];
        table_ingredients[1] = temp;
    }
}


Ingredient get_missing_ingredient() {
    int present[INGREDIENT_COUNT] = {0};
    
    present[table_ingredients[0]] = 1;
    present[table_ingredients[1]] = 1;
    
    for (int i = 0; i < INGREDIENT_COUNT; i++) {
        if (!present[i]) {
            return (Ingredient)i;
        }
    }
    return BREAD; 
}



const char* ingredient_name(Ingredient ing) {
    switch (ing) {
        case BREAD: return "Bread";
        case CHEESE: return "Cheese";
        case LETTUCE: return "Lettuce";
        default: return "Unknown";
    }
}


char maker_id(Ingredient missing_ing) {
    switch (missing_ing) {
        case BREAD: return 'A';
        case CHEESE: return 'B';
        case LETTUCE: return 'C';
        default: return '?';
    }
}