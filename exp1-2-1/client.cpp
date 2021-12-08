//clinet.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>

#define MAXLINE 4096
#define LISTENQ 1024
#define SERVER_PORT 13131
#define CLIENT_PORT 13131
#define TIMEPORT 10000

int recvline(int sock_conn, char *buf, int maxlength)
{
    int n;

    n = recv(sock_conn, buf, maxlength, 0);
    printf("Echo: %s\n", buf);
    buf[n] = 0;

    if (n == 0)
        printf("[INFO]Read Over\n"); //当读取字符为0时 表示读取完毕
    if (n < 0)
    {
        puts("[ERROR]read error\n");
        return -1;
    }

    return n;
}

int main(int argc, char **argv)
{
    int sock_client, result;
    char recvbuff[MAXLINE + 1];
    char sendbuff[MAXLINE + 1];
    char server_IP[] = "127.0.0.1";

    sock_client = socket(AF_INET, SOCK_STREAM, 0); //创建套接字
    if (sock_client < 0)
    {
        puts("[ERROR]Create Socket Failed");
        exit(-1);
    }

    struct sockaddr_in server_addr; //连接服务器
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_IP);
    if (connect(sock_client, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        puts("[ERROR]Connect failed");
        exit(-1);
    }

    //交互
    printf("> ");
    while (std::cin.getline(sendbuff, MAXLINE))
    {
        if ((*sendbuff == 'Q' || *sendbuff == 'q') && sendbuff[1] == '\n')
        {
            puts("Byebye\n");
            result = close(sock_client);
            exit(0);
        }

        result = send(sock_client, sendbuff, strlen(sendbuff), 0); //发送定长数据
        if (result < 0)
        {
            puts("[ERROR]Send Data Error!\n");
            exit(-1);
        }

        result = recvline(sock_client, recvbuff, MAXLINE); //定长接受数据
        if (result <= 0)
            exit(-1);

        memset(recvbuff, 0, sizeof(recvbuff));
        memset(sendbuff, 0, sizeof(sendbuff));
        printf("> ");
    }
}