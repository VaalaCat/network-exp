// server.c
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

void Echo(int sock, char *buff, int maxlength)
{
	int n;
	while ((n = recv(sock, buff, maxlength, 0)) > 0)
	{
		buff[n] = 0;
		std::cout << buff << std::endl;
		send(sock, buff, strlen(buff), 0);
	}

	if (n == 0)
		printf("[INFO]Finish!"); //当读取字符为0时 表示读取完毕
	else if (n < 0)
	{
		puts("[ERROR]Receive Error");
		exit(-1);
	}
}

int main()
{
	int sock_server;
	int sock_client;
	char buf[MAXLINE + 1];

	// sock_server = tcpServerInit(CLIENT_PORT);
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(CLIENT_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	sock_server = socket(AF_INET, SOCK_STREAM, 0); //创建套接字
	if (sock_server < 0)
	{
		puts("[ERROR]Socket Create Failed.");
		exit(-1);
	}
	if (bind(sock_server, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) < 0) //绑定端口地址
	{
		puts("[ERROR]Bind Failed.");
		exit(-1);
	}

	listen(sock_server, LISTENQ);

	while (1)
	{
		memset(buf, 0, sizeof(buf));
		sock_client = accept(sock_server, (struct sockaddr *)NULL, NULL);
		Echo(sock_client, buf, MAXLINE);
	}

	close(sock_server);
	return 0;
}