#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

void PRINT_ERRNO()
{
    fprintf(stderr, "\nError ocurred: %d: %s\n", errno, strerror(errno));
}

void FAIL()
{
    PRINT_ERRNO();
    exit(EXIT_FAILURE);
}

typedef struct
{
    pid_t pid;
    time_t timespamp;
    char buf[100];
} datum_t;

const char *sharedmem_name = "/shared";

int main()
{
    int shm_fd = shm_open(sharedmem_name, O_RDONLY | O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (shm_fd == -1)
    {
        fprintf(stderr, "Failed to open shared memory");
        FAIL();
    }

    if (ftruncate(shm_fd, sizeof(datum_t)) == -1)
    {
        fprintf(stderr, "Failed to ftruncate sharem mem");
        FAIL();
    }

    datum_t *datum = mmap(NULL, sizeof(datum_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (datum == MAP_FAILED)
    {
        fprintf(stderr, "Failed map shared mem");
        FAIL();
    }

    while (1)
    {
        char buf[100];
        fgets(buf, 100, stdin);
        msync(datum, sizeof(datum_t), MS_SYNC);
        printf("Datum data\n");
        printf("PID %d\n", datum->pid);
        printf("TIME %ld\n", datum->timespamp);
        printf("BUF %s\n", datum->buf);

        datum->pid = getpid();
        datum->timespamp = time(NULL);
        strcpy(datum->buf, buf);
    }

    return 0;
}
