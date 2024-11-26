/* NOTE: DO NOT REMOVE THIS COMMENT!! CSE 2431 buffer.c SP 23 08032011 */
/* STUDENT NAME: Yihone Chu */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define BUFFER_SIZE 7

typedef unsigned int buffer_item;
buffer_item buffer[BUFFER_SIZE]; /* Shared data buffer */

/* Declaration of pthread mutex locks here; these are shared by threads. */
/* See the lab instructions to find out how to initialize the locks in main, */
/* and also how to acquire the lock before entering a critical section, */
/* and how to release the lock after exiting a critical section. */
pthread_mutex_t prod_mutex; /* Shared lock used to ensure only one producer thread can access buffer at a time */
pthread_mutex_t cons_mutex; /* Shared lock used to ensure only one consumer thread can access buffer at a time */
pthread_mutex_t output_mutex; /* Shared lock used to ensure only one thread can write output at any time */

/* See the lab instructions to find out how to initialize the semaphores in main. */
sem_t empty; /* Shared data empty counting semaphore */
sem_t full;  /* Shared data full counting semaphore */

/* Shared data buffer indexes */
int in = 0;  /* Shared data/index for producers to insert item into buffer */
int out = 0; /* Shared data/index for consumers to remove item from buffer */

/* Random number generator seed value */
unsigned int seed = 100; /* Seed value for rand_r; initialized to 100 */
unsigned int* seedp = &seed; /* Pointer to seed for rand_r(), a re-entrant and thread-safe version of rand() */

/* Function declarations */
void* producer(void* param);
void* consumer(void* param);
void insert_item(buffer_item item);
void remove_item(buffer_item* item);

/* Function shared by all producer threads; DO NOT CHANGE THIS CODE */
void* producer(void* param) {
    buffer_item rand;
    while (1) {
        /* Generate a random number between 0 and 99 */
        rand = rand_r(seedp) % 100;
        /* Sleep for rand * 10000 microseconds */
        usleep(rand * 10000);
        /* Insert item into buffer */
        insert_item(rand);
    }
}

/* Function shared by all consumer threads; DO NOT CHANGE THIS CODE */
void* consumer(void* param) {
    buffer_item rand;
    while (1) {
        /* Generate a random number between 0 and 99 */
        rand = rand_r(seedp) % 100;
        /* Sleep for rand * 10000 microseconds */
        usleep(rand * 10000);
        /* Remove item from buffer */
        remove_item(&rand);
    }
}

/* Insert a single item into the buffer */
void insert_item(buffer_item item) {
    /* Wait until there is space in the buffer */
    sem_wait(&empty);

    /* Acquire producer lock */
    pthread_mutex_lock(&prod_mutex);
    /* Insert the item into the buffer */
    buffer[in] = item;
    in = (in + 1) % BUFFER_SIZE;
    /* Release producer lock */
    pthread_mutex_unlock(&prod_mutex);

    /* Signal that there is a new item in the buffer */
    sem_post(&full);

    /* Output mutex lock should be held before writing output */
    pthread_mutex_lock(&output_mutex);
    printf("Producer produced %d\n", item);
    pthread_mutex_unlock(&output_mutex);
}

/* Remove a single item from the buffer */
void remove_item(buffer_item* item) {
    /* Wait until there is at least one item in the buffer */
    sem_wait(&full);

    /* Acquire consumer lock */
    pthread_mutex_lock(&cons_mutex);
    /* Remove the item from the buffer */
    *item = buffer[out];
    out = (out + 1) % BUFFER_SIZE;
    /* Release consumer lock */
    pthread_mutex_unlock(&cons_mutex);

    /* Signal that there is space available in the buffer */
    sem_post(&empty);

    /* Output mutex lock should be held before writing output */
    pthread_mutex_lock(&output_mutex);
    printf("\tConsumer removed %d.\n", *item);
    pthread_mutex_unlock(&output_mutex);
}

/* Main function to initialize the buffers, semaphores, and threads */
int main(int argc, char* argv[]) {
    int sleep_time = atoi(argv[1]);
    int num_prod_threads = atoi(argv[2]);
    int num_cons_threads = atoi(argv[3]);
    int i;

    /* Initialize mutex locks */
    pthread_mutex_init(&prod_mutex, NULL);
    pthread_mutex_init(&cons_mutex, NULL);
    pthread_mutex_init(&output_mutex, NULL);

    /* Initialize semaphores */
    sem_init(&full, 0, 0);       /* Semaphore for full buffer */
    sem_init(&empty, 0, BUFFER_SIZE); /* Semaphore for empty buffer */

    /* Create thread attributes */
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    /* Create producer threads */
    for (i = 0; i < num_prod_threads; i++) {
        pthread_create(&tid, &attr, producer, NULL);
    }

    /* Create consumer threads */
    for (i = 0; i < num_cons_threads; i++) {
        pthread_create(&tid, &attr, consumer, NULL);
    }

    /* Sleep for the specified number of seconds */
    sleep(sleep_time);

    /* Cleanup and exit */
    return 0;
}
