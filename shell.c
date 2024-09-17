// shell example

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
