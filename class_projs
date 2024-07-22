// aardvarks.c

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

/////////////////////////////////
/////////////////////////////////

// timing.c

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

#define NUM_ITERATIONS 100000
double rusage_empty_loop_time = 0;
double gettime_empty_loop_time = 0;

void rusage_loop() {
    struct rusage start, end;
    getrusage(RUSAGE_SELF, &start);
    for (int i = 0; i < NUM_ITERATIONS; ++i){}
    getrusage(RUSAGE_SELF, &end);
    rusage_empty_loop_time += (end.ru_stime.tv_sec - start.ru_stime.tv_sec) * 1000000LL + (end.ru_stime.tv_usec - start.ru_stime.tv_usec);
    rusage_empty_loop_time = (double)((end.ru_stime.tv_sec - start.ru_stime.tv_sec) * 1000000LL + (end.ru_stime.tv_usec - start.ru_stime.tv_usec)) / NUM_ITERATIONS / 1e6;
    printf("rusage empty loop time: %.12e seconds\n", rusage_empty_loop_time);
}

void gettime_loop() {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_ITERATIONS; ++i){}
    gettimeofday(&end, NULL);
    gettime_empty_loop_time = (end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec);
    gettime_empty_loop_time = (double)((end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec)) / NUM_ITERATIONS / 1e6;
    printf("gettime empty loop time: %.10e seconds\n", gettime_empty_loop_time);
}

void mmap_avg() {
    struct rusage start, end;
    double system_latency = 0, user_latency = 0;
    //iterate for average
    getrusage(RUSAGE_SELF, &start);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        void* ptr = mmap(NULL, 4096, PROT_READ, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
        //error check
        if (ptr == MAP_FAILED){
            perror("mmap failure");
            exit(EXIT_FAILURE);
        }
        //munmap(ptr, 4096);    
    }
    getrusage(RUSAGE_SELF, &end);
    // calculate and print system and user time
    system_latency = (double)((end.ru_stime.tv_sec - start.ru_stime.tv_sec) * 1000000LL + (end.ru_stime.tv_usec - start.ru_stime.tv_usec)) - rusage_empty_loop_time;
    user_latency = (double) ((end.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000LL + (end.ru_utime.tv_usec - start.ru_utime.tv_usec)) - rusage_empty_loop_time;
    printf("mmap system time average: %.2f microsecond(s)\n", system_latency / NUM_ITERATIONS);
    printf("mmap user time average: %.2f microsecond(s)\n", user_latency / NUM_ITERATIONS);
}

pthread_mutex_t mutexes[NUM_ITERATIONS];
void mutex_lock_avg() {
    struct rusage start, end;
    double system_latency = 0, user_latency = 0;
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        pthread_mutex_init(&mutexes[i], NULL);
    }
    //iterate for average
    getrusage(RUSAGE_SELF, &start);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        int mut = pthread_mutex_lock(&mutexes[i]);
        //error check
        if (mut != 0){
            perror("Mutex lock failure");
            exit(EXIT_FAILURE);
        }            
    }
    getrusage(RUSAGE_SELF, &end);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        int mut = pthread_mutex_unlock(&mutexes[i]);
        //error check
        if (mut != 0){
            perror("Mutex unlock failure");
            exit(EXIT_FAILURE);
        }
        pthread_mutex_destroy(&mutexes[i]);
    }
    // calculate and print system and user time
    system_latency = (double) ((end.ru_stime.tv_sec - start.ru_stime.tv_sec) * 1000000LL + (end.ru_stime.tv_usec - start.ru_stime.tv_usec)) - rusage_empty_loop_time;
    user_latency = (double) ((end.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000LL + (end.ru_utime.tv_usec - start.ru_utime.tv_usec)) - rusage_empty_loop_time;
    system_latency = (system_latency < 0) ? 0 : system_latency;
    user_latency = (user_latency < 0) ? 0 : user_latency;
    printf("pthread_mutex_lock average system time: %.30f microsecond(s)\n", system_latency / NUM_ITERATIONS);
    printf("pthread_mutex_lock average user time: %.5f microsecond(s)\n", user_latency / NUM_ITERATIONS);
}

void direct_read_write_avg() {
    struct timeval start, end;
    double write_latency = 0, read_latency = 0;
    //create file in tmp
    int fd = open("/tmp/jboyd06", O_DIRECT|O_CREAT|O_WRONLY|O_SYNC, 0666);
    //error check
    if (fd == -1) {
        perror("File could not be opened");
        exit(EXIT_FAILURE);
    }
    //align memory buffer
    void *buf;
    //error check
    if (posix_memalign(&buf, 4096, 4096) != 0) {
        perror("Memory could not be aligned");
        exit(EXIT_FAILURE);
    }
    //iterate for average direct write
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        ssize_t write_num = write(fd, buf, 4096);
        //error check
        if (write_num == -1){
            perror("Direct write failure");
            exit(EXIT_FAILURE);
        }
    }
    gettimeofday(&end, NULL);
    write_latency = (double) ((end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec)) - gettime_empty_loop_time;
    if (close(fd) == -1){
        //error check
        perror("Direct write close failure");
    }
    fd = open("/tmp/jboyd06", O_DIRECT|O_RDONLY);
    //error check
    if (fd == -1) {
        perror("File could not be opened for reading");
        exit(EXIT_FAILURE);
    }
    //iterate for average direct read
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        ssize_t read_num = read(fd, buf, 4096);
        //error check
        if (read_num == -1){
            perror("Direct read failure");
            exit(EXIT_FAILURE);
        }
    }
    gettimeofday(&end, NULL);
    read_latency = (double) ((end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec)) - gettime_empty_loop_time;
    //error check
    if (close(fd) == -1){
        perror("Direct read close failure");
    }
    free(buf);
    unlink("/tmp/jboyd06");
    printf("Direct write average time: %.2f microsecond(s)\n", write_latency / NUM_ITERATIONS);
    printf("Direct read average time: %.2f microsecond(s)\n", read_latency / NUM_ITERATIONS);
}

void cache_read_write_avg() {
    struct timeval start, end;
    double write_latency = 0, read_latency = 0;
    //create file in tmp
    int fd = open("/tmp/jboyd06ca$h", O_CREAT|O_WRONLY, 0666);
    //error check
    if (fd == -1) {
        perror("Cached file could not be opened");
        exit(EXIT_FAILURE);
    }
    //create buffer
    char* buf = malloc(4096);
    //error check
    if (!buf) {
        perror("Memory could not be allocated for buffer");
        exit(EXIT_FAILURE);
    }
    //iterate for average write
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        ssize_t write_num = write(fd, buf, 4096);
        //error check
        if (write_num == -1){
            perror("Cache write failure");
            exit(EXIT_FAILURE);
        }
    }
    gettimeofday(&end, NULL);
    write_latency = (double) ((end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec)) - gettime_empty_loop_time;
    //error check
    if (close(fd) == -1){
        perror("Cache write close failure");
    }
    fd = open("/tmp/jboyd06ca$h", O_RDONLY);
    //error check
    if (fd == -1) {
        perror("Error opening cached file for reading");
        exit(EXIT_FAILURE);
    }
    //iterate for average read
    gettimeofday(&start, NULL);
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        ssize_t read_num = read(fd, buf, 4096);
        //error check
        if (read_num == -1){
            perror("Cache read failure");
            exit(EXIT_FAILURE);
        }
    }
    gettimeofday(&end, NULL);
    read_latency = (double) ((end.tv_sec - start.tv_sec) * 1000000LL + (end.tv_usec - start.tv_usec)) - gettime_empty_loop_time;
    //error check
    if (close(fd) == -1){
        perror("Cache read close failure");
    }
    free(buf);
    unlink("/tmp/jboyd06ca$h");
    printf("Cache write average time: %.2f microsecond(s)\n", write_latency / NUM_ITERATIONS);
    printf("Cache read average time: %.2f microsecond(s)\n", read_latency / NUM_ITERATIONS);
}
//run measurements
int main() {
    rusage_loop();
    gettime_loop();
    mmap_avg();
    mutex_lock_avg();
    direct_read_write_avg();
    cache_read_write_avg();
    return 0;
}

/////////////////////////////////
/////////////////////////////////

// shell.c

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    //set a variable to store user input
    char input[255];
    //loop shell prompt infinitely until "exit" typed
    while (1) {
        //prompt user
        printf("jsh$ ");
        //read user input & remove newline character at end of input
        if (fgets(input, 255, stdin) == NULL) {
            printf("fgets() failure\n");
            exit(EXIT_FAILURE);
        }
        input[strcspn(input, "\n")] = '\0';
        //if "exit" typed
        if (strcmp(input, "exit") == 0) {
            printf("jsh Exiting...\n");
            break;
        }
        //separate input by spaces to be used as args by execvp
        char* processes[255];
        int num_pipes = 0;
        char* process;
        char* str = input;
        while ((process = strtok_r(str, "|", &str))) {
            processes[num_pipes++] = process;
        }
        int pipefd[num_pipes - 1][2];
        for (int i = 0; i < num_pipes - 1; i++) {
            if (pipe(pipefd[i]) == -1) {
                perror("pipe() failure");
                exit(EXIT_FAILURE);
            }
        }
        //fork & pipe
        pid_t last_pid = 0;
        for (int i = 0; i < num_pipes; i++) {
            char* input_args[255];
            int j = 0;
            char* word = strtok(processes[i], " ");
            while (word != NULL) {
                input_args[j++] = word;
                word = strtok(NULL, " ");
            }
            input_args[j] = NULL;
            pid_t pid = fork(); 
            //fork error handling
            if (pid < 0) {
                perror("fork() failure");
                exit(EXIT_FAILURE);
            } else if (pid == 0) { //duplicate process then close pipe
                if (i > 0) {
                    if (dup2(pipefd[i - 1][0], STDIN_FILENO) == -1) {
                        perror("dup2() failure");
                        exit(EXIT_FAILURE);
                    }
                    close(pipefd[i - 1][1]); 
                }
                if (i < num_pipes - 1) {
                    if (dup2(pipefd[i][1], STDOUT_FILENO) == -1) {
                        perror("dup2() failure");
                        exit(EXIT_FAILURE);
                    }
                    close(pipefd[i][0]);
                }
                //close file descriptors
                for (int k = 0; k < num_pipes - 1; k++){
                    close(pipefd[k][0]);
                    close(pipefd[k][1]);
                }
                if (execvp(input_args[0], input_args) == -1 && strcmp(input, "exit") != 0){
                    perror("execvp() failure");
                    exit(EXIT_FAILURE);
                }
            } else {
                last_pid = pid;
            }
        }
        for (int i = 0; i < num_pipes - 1; i++) {
            close(pipefd[i][0]);
            close(pipefd[i][1]);
        }
        //wait on all processes
        int status;
        for (int i = 0; i < num_pipes; i++) {
            if (i == num_pipes - 1){
                waitpid(last_pid, &status, 0);
                if (WIFEXITED(status)) {
                    printf("jsh status: %d\n", WEXITSTATUS(status));
                }
            } else {
                wait(&status);
            }
        }
    }
    //program ran successfully
    return(0);
}
