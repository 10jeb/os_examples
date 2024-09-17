//dining philosophers concurrency solution with aardvarks instead

#include "/comp/111m1/assignments/aardvarks/anthills.h"
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

int initialized = FALSE;
pthread_mutex_t init_lock = PTHREAD_MUTEX_INITIALIZER; 
sem_t aardvarks_per_hill_sem[ANTHILLS];
sem_t ants_per_hill_sem[ANTHILLS];
int ants_per_hill[ANTHILLS];

//struct to hold thread data
struct thread_data {
    sem_t* semaphore;
    double start_time;
};

//detached thread to post the semaphore
void *swallow_posting(void *arg) {
    struct thread_data* data = (struct thread_data*)arg;
    while (elapsed() - data->start_time < 1.5){}
    sem_post(data->semaphore);
    pthread_exit(NULL);
}

void *aardvark(void *input) {
    char aname = *(char *)input;
    //initialize anthill semaphores and variables
    pthread_mutex_lock(&init_lock);
    if (!initialized++) {
        for (int i = 0; i < ANTHILLS; i++) {
            sem_init(&aardvarks_per_hill_sem[i], 0, AARDVARKS_PER_HILL);
            sem_init(&ants_per_hill_sem[i], 0, ANTS_PER_HILL);
        }
    }
    pthread_mutex_unlock(&init_lock);
    //slurp ants in orderly fashion while they remain
    while (chow_time()) {
        //determine from which anthill to slurp
        int anthill = rand() % ANTHILLS;
        //slurp if possible and when able
        if (sem_trywait(&ants_per_hill_sem[anthill]) == 0) {
            if (sem_trywait(&aardvarks_per_hill_sem[anthill]) == 0) {
                //create thread to detach to call sem_post 1 second after slurp is called
                pthread_t sem_post;
                struct thread_data data;
                data.semaphore = &aardvarks_per_hill_sem[anthill];
                data.start_time = elapsed(); 
                pthread_create(&sem_post, NULL, swallow_posting, (void*)&data);
                pthread_detach(sem_post);
                //slurp and decrement ant count
                slurp(aname, anthill);
            } else {
                //release semaphore if unable to gain access to hill
                sem_post(&ants_per_hill_sem[anthill]);
            }
        }
    }
    return NULL;
}
