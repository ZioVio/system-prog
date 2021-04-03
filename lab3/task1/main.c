#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

int get_write_fd(const char *path)
{
    const int flags = O_CREAT | O_WRONLY;
    const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int file = open(path, flags, mode);
    return file;
}

int log_fd;
const char *log_path = "log.txt";

void signal_handler(int signo, siginfo_t *si, void *ucontext);

int main()
{
    log_fd = get_write_fd(log_path);
    char buf[100];
    snprintf(buf, 30, "Start with PID: %d\n", getpid());
    write(log_fd, buf, strlen(buf));

    struct sigaction old_action;
    struct sigaction new_action;
    sigaction(SIGHUP, NULL, &old_action);
    new_action.sa_sigaction = signal_handler;
    new_action.sa_flags = SA_SIGINFO;
    sigaction(SIGHUP, &new_action, NULL);

    int i = 0;
    while (1) {
        if (i > __INT_MAX__) {
            i = 0;
        }
        sleep(5);
        snprintf(buf, 20, "In cycle %d\n", i++);
        write(log_fd, buf, strlen(buf));
    }
    
    close(log_fd);
    return 0;
}

void signal_handler(int signo, siginfo_t *si, void *ucontext)
{
    char buf[300];
    snprintf(buf, 30, "code: %d\n", si->si_code);
    write(log_fd, buf, strlen(buf));
    snprintf(buf, 30, "erno: %d, %s\n", si->si_errno, strerror(si->si_errno));
    write(log_fd, buf, strlen(buf));
    snprintf(buf, 30, "pid: %d\n", si->si_pid);
    write(log_fd, buf, strlen(buf));
    snprintf(buf, 30, "uid: %d\n", si->si_uid);
    write(log_fd, buf, strlen(buf));
    snprintf(buf, 30, "status: %d\n", si->si_status);
    write(log_fd, buf, strlen(buf));
    snprintf(buf, 30, "band: %ld\n", si->si_band);
    write(log_fd, buf, strlen(buf));
}