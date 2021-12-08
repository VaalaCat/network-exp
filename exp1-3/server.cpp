// server.c
// #include <stdio.h>
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

int Echo(char *buff, int len)
{
	int n = len;
	buff[n] = 0;
	int a = 0;
	int b = 0;
	int i = 0;
	for (i = 0; i < n; i++)
	{
		if (buff[i] == ' ')
			break;
		a += buff[i] - '0';
		a *= 10;
	}
	a /= 10;
	for (; i < n; i++)
	{
		if (buff[i] == ' ')
			continue;
		b += buff[i] - '0';
		b *= 10;
	}
	b /= 10;
	return a + b;
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
		int n = 0;
		while ((n = recv(sock_client, buf, MAXLINE, 0)) > 0)
		{
			int a = Echo(buf, n);
			char tmpbuffer[MAXLINE + 1];
			sprintf(tmpbuffer, "%d", a);
			std::cout << tmpbuffer << std::endl;
			send(sock_client, tmpbuffer, strlen(tmpbuffer), 0);
		}
	}
	close(sock_server);
	return 0;
}