#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <string.h>
#include <strings.h>

#define SERV_PORT 1234
#define MAXLINE 1024

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    int n;
    char recvBuf[MAXLINE+1], sendBuf[MAXLINE];

    while(fgets(sendBuf, MAXLINE, stdin) != NULL)
    {
        sendto(sockfd, sendBuf, strlen(sendBuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        n = recvfrom(sockfd, recvBuf, MAXLINE, 0, NULL, NULL);
        recvBuf[n] = '\0';
        fputs(recvBuf, stdout);
        if (!strcmp(sendBuf, "q\n"))
            break;
    }
}
