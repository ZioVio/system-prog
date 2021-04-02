#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define ID_ERR -1
#define LOOP_COUNT 2

void PRINT_ERRNO() {
    fprintf(stderr, "\nError ocurred: %d: %s\n", errno, strerror(errno));
}

void FAIL() {
    PRINT_ERRNO();
    exit(EXIT_FAILURE);
}

void print_current_proccess_details();
void fork_and_process_results();

void wait_for_child_proc();
void print_ids();

int main() {
    print_current_proccess_details();
    fork_and_process_results();
    return 0;
}

void print_current_proccess_details() {
    pid_t   pid = getpid(), 
            gid = getgid(), 
            sid = getsid(pid),
            ppid = getppid();
    if (sid == ID_ERR) {
        fprintf(stderr, "Failed to get sid\n");
        FAIL();
    }
    printf("Current proccess details:\n");
    printf("    Proccess id: %d\n", pid);
    printf("    Parent proccess id: %d\n", ppid);
    printf("    Group id: %d\n", gid);
    printf("    Session id: %d\n", sid);
}

void fork_and_process_results() {
    pid_t fid = fork();
    if (fid == ID_ERR) {
        fprintf(stderr, "Fork error\n");
        FAIL();
    }
    else if (fid == 0) {
        // new proccess
        print_ids();
        printf("Child proccess finished\n");
    } 
    else {
        // parent proccess
        print_ids();
        wait_for_child_proc();
        printf("Parent waited for child proccess finished\n");
    }
}

void wait_for_child_proc() {
    pid_t cpid = wait(NULL);
    if (cpid == ID_ERR) {
        fprintf(stderr, "Error waiting for child proccess to finish");
        FAIL();
    }
}
void print_ids() {
    for (int i = 0; i < LOOP_COUNT; i++) {
        print_current_proccess_details();
    }   
}