#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 1234
#define MAXDATASIZE 100

int main()
{
    int sockfd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t addrlen;
    int num;
    char recvBuf[MAXDATASIZE];
    char sendBuf[MAXDATASIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("[ERROR]Creat socket Failed");
        exit(1);
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
    {
        perror("[ERROR]Bind ERROR");
        exit(1);
    }

    addrlen = sizeof(client);

    while (1)
    {
        num = recvfrom(sockfd, recvBuf, MAXDATASIZE, 0, (struct sockaddr *)&client, &addrlen);

        if (num < 0)
        {
            perror("[ERROR]Recv ERROR");
            exit(1);
        }

        recvBuf[num] = '\0';
        printf("%s", recvBuf);
        sprintf(sendBuf, "echo: %s\n", recvBuf);
        sendto(sockfd, sendBuf, 22, 0, (struct sockaddr *)&client, addrlen);
        if (!strcmp(recvBuf, "q\n"))
            break;
    }
    close(sockfd);
}
