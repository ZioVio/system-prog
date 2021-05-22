#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFLEN 1024
#define CLIENT_EXIT_COMMAND "CLOSE"

void parent_proc(int fd);
void child_proc(int fd);
void on_client(int client_socket_fd, int fd);

void PRINT_ERRNO()
{
    fprintf(stderr, "\nError ocurred: %d: %s\n", errno, strerror(errno));
}

void FAIL()
{
    PRINT_ERRNO();
    exit(EXIT_FAILURE);
}

const char *LOG_FILENAME = "file.log";

int main(void)
{
    int fd = open(LOG_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        fprintf(stderr, "Error while opening %s for writing (initial)\n", LOG_FILENAME);
        FAIL();
    }
    const char *startlog = "Server started\n";
    write(fd, startlog, strlen(startlog));

    pid_t fork_id = fork();
    if (fork_id < 0)
    {
        fprintf(stderr, "Failed to fork");
        close(fd);
        FAIL();
    }
    else if (fork_id == 0)
    {
        child_proc(fd);
    }
    else
    {
        parent_proc(fd);
    }

    // End of program
    close(fd);
    printf("\n\nChild proc server - Main: I'm done\n");
    return 0;
}

void parent_proc(int fd)
{
    const char *msg = "Create child, exiting\n";
    write(fd, msg, strlen(msg));
    close(fd);
    exit(EXIT_SUCCESS);
}

void child_proc(int fd)
{
    int newsid = setsid();
    char buf[BUFLEN];
    struct sockaddr_in sa_in;
    int sockfd;
    int client_socket_fd;
    struct sockaddr_in client_sa_in;
    int client_sa_in_size;

    if (newsid == -1)
    {
        printf("Error while calling setsid()\n");
    }

    sa_in.sin_family = PF_INET;
    sa_in.sin_addr.s_addr = htonl(INADDR_ANY);
    sa_in.sin_port = htons(3216);

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        const char *msg = "Child proc server - Main: ERROR when opening socket\n";
        write(fd, msg, strlen(msg));
        close(fd);
        FAIL();
    }
    if (bind(sockfd, (struct sockaddr *)&sa_in, sizeof(sa_in)) == -1)
    {
        const char *msg = "Child proc server - Main: ERROR when binding\n";
        write(fd, msg, strlen(msg));
        close(fd);
        FAIL();
    }

    sprintf(buf, "[%d] Child proc server - Main: Starting listening\n", getpid());
    write(fd, buf, strlen(buf));

    listen(sockfd, 5);
    client_sa_in_size = sizeof(client_sa_in);

    while (1)
    {
        client_socket_fd = accept(sockfd, (struct sockaddr *)&client_sa_in, &client_sa_in_size);
        if (client_socket_fd < 0)
        {
            const char *msg = "Child proc server - Main: failed to establish connection\n";
            write(fd, msg, strlen(msg));
        }
        else
        {
            pid_t fork_id = fork();
            if (fork_id < 0)
            {
                const char *msg = "Child proc server: failed to fork";
                write(fd, msg, strlen(msg));
                close(client_socket_fd);
                FAIL();
            }
            else if (fork_id == 0)
            {
                close(sockfd);
                on_client(client_socket_fd, fd);
                close(fd);
                exit(EXIT_SUCCESS);
            }
            else
            {
                const char *msg = "Child proc server - Main: forked client finished\n";
                write(fd, msg, strlen(msg));
                close(client_socket_fd);
            }
        }
    }

    close(fd);
}

void on_client(int client_socket_fd, int fd)
{
    int newSid = setsid();
    char recv_buff[BUFLEN];
    int recvbytes;
    char sendbuff[BUFLEN];
    int sendbytes;
    time_t rawtime;
    struct tm cur_time;
    int is_server_running = 1;

    if (newSid == -1)
    {
        printf("Error while calling setsid()\n");
    }

    do
    {
        recvbytes = recv(client_socket_fd, recv_buff, sizeof(recv_buff) - 1, 0);
        recv_buff[recvbytes] = '\0';
        if (recvbytes <= 0)
        {
            const char *msg = "Server fork: no message on request\n";
            write(fd, msg, strlen(msg));
            continue;
        }

        time(&rawtime);
        cur_time = *localtime(&rawtime);
        sendbytes = sprintf(sendbuff, "PID: [%d] on %s\nMessage:%s", getpid(), asctime(&cur_time), recv_buff);
        if (send(client_socket_fd, sendbuff, sendbytes, 0) == -1)
        {
            const char *msg = "Server fork: failed to send data to client\n";
            write(fd, msg, strlen(msg));
        }
        is_server_running = strcmp(recv_buff, CLIENT_EXIT_COMMAND) != 0;
    } while (is_server_running);

    const char *msg = "Server fork: CLOSE\n";
    write(fd, msg, strlen(msg));
    close(client_socket_fd);
}