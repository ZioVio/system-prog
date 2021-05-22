#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFLEN 1024

void PRINT_ERRNO()
{
    fprintf(stderr, "\nError ocurred: %d: %s\n", errno, strerror(errno));
}

void FAIL()
{
    PRINT_ERRNO();
    exit(EXIT_FAILURE);
}


int main(void) {
    struct sockaddr_in sa_in;
    int sockfd;
    char send_buff[BUFLEN];
    int send_bytes;
    char recv_buff[BUFLEN];
    int recv_bytes;


    sa_in.sin_family = PF_INET;
    sa_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    sa_in.sin_port = htons(3216);

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to open socket\n");
        FAIL();
    }

    if (connect(sockfd, (struct sockaddr *)&sa_in, sizeof(sa_in)) == -1) {
        fprintf(stderr, "Failed to connect\n");
        FAIL();
    }

    do {
        fgets(send_buff, BUFLEN, stdin);
        send_bytes = strlen(send_buff);

        if (send(sockfd, send_buff, send_bytes, 0) == -1) {
            fprintf(stderr, "Failed to send message to server");
            FAIL();
        }

        recv_bytes = recv(sockfd, recv_buff, sizeof(recv_buff) - 1, 0);
        recv_buff[recv_bytes] = '\0';
        printf("Response from server: \n%s\n", recv_buff);
    } while(strcmp(send_buff, "close\n") != 0);

    close(sockfd);
    return 0;
}
