#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 5
#define SELECT_ERROR -1
#define SELECT_NO_DATA_ERROR 0

void PRINT_ERRNO() {
    fprintf(stderr, "\nError ocurred: %d: %s\n", errno, strerror(errno));
}

void FAIL() {
    PRINT_ERRNO();
    exit(EXIT_FAILURE);
}

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Expected at least 2 arguments\n");
        FAIL();
    }
    const char * input_tag = argv[1];

    fd_set readfds;
    struct timeval timeval;
    char buf[BUFFER_SIZE] = {0};

    for (;;) {
        timeval.tv_sec = TIMEOUT_SEC;
        timeval.tv_usec = 0;

        FD_ZERO(&readfds);

        FD_SET(STDIN_FILENO, &readfds);

        int select_result = select(
            STDOUT_FILENO, 
            &readfds, 
            NULL, 
            NULL, 
            &timeval
        );

        if (select_result == SELECT_ERROR) {
            fprintf(stdout, "Error calling select");
            FAIL();
        }

        if (select_result == SELECT_NO_DATA_ERROR) {
            printf("Tag \"%s\"\n", input_tag);
            printf("Call select again\n");
            continue;
        }

        read(STDIN_FILENO, &buf, BUFFER_SIZE);
        printf("Tag: \"%s\"\n", input_tag);
        printf("Input data: \"%s\"\n", buf);
        break;
    }

    return 0;
}   