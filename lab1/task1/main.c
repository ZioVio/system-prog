#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define OPEN_FILE_ERROR -1
#define BUFFER_SIZE 512

void PRINT_ERRNO() {
    fprintf(stderr, "\nError ocurred: %d: %s\n", errno, strerror(errno));
}

void FAIL() {
    PRINT_ERRNO();
    exit(EXIT_FAILURE);
}


int get_read_fd(const char * path);
int get_write_fd(const char * path);
void process_files(int input_fd, int ouput_fd);

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Expected at least 3 arguments\n");
        FAIL();
    }
    const char * input_file_name = argv[1];
    const char * output_file_name = argv[2];

    const int read_fd = get_read_fd(input_file_name);
    if (read_fd == OPEN_FILE_ERROR) {
        fprintf(stderr, "Failed to open file %s\n", input_file_name);
        FAIL();
    }
    const int write_fd = get_write_fd(output_file_name);
    if (write_fd == OPEN_FILE_ERROR) {
        fprintf(stderr, "Failed to open or create file %s\n", output_file_name);
        FAIL();
    }
    process_files(read_fd, write_fd);
    close(read_fd);
    close(write_fd);
    return 0;
}

void process_files(int input_fd, int ouput_fd) {
    char buf[BUFFER_SIZE] = {0};
    long read_on_iteration = 0,
        total_read_bytes = 0, 
        total_written_bytes = 0;

    do {
        read_on_iteration = read(input_fd, &buf, BUFFER_SIZE);
        if (read_on_iteration < 0) {
            fprintf(stderr, "Some error ocureed while reading file");
            FAIL();
        }

        total_read_bytes += read_on_iteration;
        
        for (long i = 0; i < read_on_iteration; i++) {
            buf[i] = toupper(buf[i]);
        }

        long written_on_iteration = write(ouput_fd, &buf, read_on_iteration);
        if (written_on_iteration < 0) {
            fprintf(stderr, "Failed to write file");
            FAIL();
        }
        total_written_bytes += written_on_iteration;

        if (written_on_iteration != read_on_iteration) {
            fprintf(stderr, "Read and written bytes count mismatch: \n");
            fprintf(stderr, "Read: %ld: Written %ld", read_on_iteration, written_on_iteration);
            FAIL();
        }
    } while (read_on_iteration > 0);

    char total_info_buffer[BUFFER_SIZE] = {' '};
    sprintf(
        total_info_buffer, 
        "\nTotal bytes read: %ld\nTotal bytes written: %ld\n", 
        total_read_bytes,
        total_written_bytes 
    );

    int total_info_written_length = write(ouput_fd, total_info_buffer, BUFFER_SIZE);
    if (total_info_written_length < 0) {
        fprintf(stderr, "Failed to write total info");
        FAIL();
    }
}

int get_read_fd(const char *path) {
    const int flags = O_RDONLY;
    int file = open(path, flags);
    return file;
}

int get_write_fd(const char *path) {
    const int flags =  O_CREAT | O_WRONLY;
    const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;  
    int file = open(path, flags, mode);
    return file;
}
