#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_DESRIPTORS 1024

void PRINT_ERRNO() {
    fprintf(stderr, "\nError ocurred: %d: %s\n", errno, strerror(errno));
}

void FAIL() {
    PRINT_ERRNO();
    exit(EXIT_FAILURE);
}

int get_write_fd(const char *path) {
    const int flags =  O_CREAT | O_WRONLY;
    const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;  
    int file = open(path, flags, mode);
    return file;
}

const char * log_file_path = "file.log";
const char * start_message = "START\n";

// returns file descriptor
int log_start_message();
void on_child_proc();

int main(void) {
    int fd = log_start_message();

    pid_t fid = fork();

    if (fid == -1) {
        fprintf(stderr, "Error ocurred on fork");
        close(fd);
        FAIL();
    }
    else if (fid == 0) {
        // new proc
        on_child_proc();
    } 
    else {
        const char * message = "Child proccess created";
        puts(message);
        write(fd, message, strlen(message));
        close(fd);
        exit(EXIT_SUCCESS);
    }
    return 0;
}


int log_start_message() {
    int fd = get_write_fd(log_file_path);
    if (fd == -1) {
        fprintf(stderr, "Failed to open %s to read", log_file_path);
        FAIL();
    }

    write(fd, start_message, strlen(start_message)); // ignore err for this case
    return fd;
}

void on_child_proc() {
    pid_t newsid = setsid();
    if (newsid == -1) {
        fprintf(stderr, "Failed to setsid");
        FAIL();
    }

    int chdir_result = chdir("/");
    if (chdir_result == -1) {
        fprintf(stderr, "Failed to change dorwing dir");
        FAIL();
    }

    for (int i = 0; i < MAX_DESRIPTORS; i++) {
        close(i);
    }

    // https://stackoverflow.com/questions/4263173/redirecting-stdin-stdout-stderr-to-dev-null-in-c
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);  

    char buf[100] = {0};

    int buflen = sprintf(buf, "PID: %d, GID: %d, SID: %d\n", getpid(), getgid(), newsid);
    int fd = get_write_fd(log_file_path);

    if (fd == -1) {
      fprintf(stderr, "failed to open log from deamon");
      FAIL();
    }

    write(fd, buf, buflen);
    close(fd);

    int i = 0;
    while(1) {
        if (i >= __INT_MAX__) {
            i = 0;
        }
        printf("Iteration: %d", i++);
    }
}