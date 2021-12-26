//server.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

  int port = PORT;
  while (1)
  {
    num = recvfrom(sockfd, recvBuf, MAXDATASIZE, 0, (struct sockaddr *)&client, &addrlen);

    if (num < 0)
    {
      perror("[ERROR]Recv ERROR");
      exit(1);
    }
    
    recvBuf[num] = 0;
    char newPort[10];
    sprintf(newPort, "%d", ++port);
    newPort[strlen(newPort)] = 0;

    sendto(sockfd, newPort, 22, 0, (struct sockaddr *)&client, addrlen);

    pid_t child = fork();
    if (!child)
    {
      int Nsockfd;
      struct sockaddr_in Nserver;
      struct sockaddr_in Nclient;
      // socklen_t Naddrlen;
      int Nnum = num;
      char NrecvBuf[MAXDATASIZE] = {0};
      char NsendBuf[MAXDATASIZE] = {0};
      memcpy(NrecvBuf, recvBuf, Nnum);
      memcpy(&Nclient, &client, sizeof(client));
      
      if ((Nsockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
      {
        perror("[ERROR]Creat Nsocket Failed");
        exit(1);
      }

      bzero(&Nserver, sizeof(Nserver));
      Nserver.sin_family = AF_INET;
      Nserver.sin_port = htons(port);
      Nserver.sin_addr.s_addr = inet_addr("127.0.0.1");

      if (bind(Nsockfd, (struct sockaddr *)&Nserver, sizeof(Nserver)) == -1)
      {
        perror("[ERROR]Bind ERROR");
        exit(1);
      }

      while (1)
      {
        sprintf(NsendBuf, "echo: %s\n", NrecvBuf);
        sendto(Nsockfd, NsendBuf, 22, 0, (struct sockaddr *)&client, addrlen);
        if (!strcmp(NrecvBuf, "q\n"))
          break;

        bzero(&NrecvBuf, MAXDATASIZE);
        bzero(&NsendBuf, MAXDATASIZE);

        num = recvfrom(Nsockfd, NrecvBuf, MAXDATASIZE, 0, (struct sockaddr *)&client, &addrlen);

        if (num < 0)
        {
          perror("[ERROR]Recv ERROR");
          exit(1);
        }
      }
    }
  }
  close(sockfd);
}