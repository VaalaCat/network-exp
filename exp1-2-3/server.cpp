// Server.c
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LISTENQ 1024
#define SERV_PORT 9999
#define BUFSIZE 0x1000

void echo(int client)
{
	ssize_t n;
	char buf[BUFSIZE];
	while ((n = read(client, buf, BUFSIZE)) > 0)
		write(client, buf, n);
}

int main(int argc, char **argv)
{
	int client, server;
	struct sockaddr_in cliaddr, servaddr;
	socklen_t clilen;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server == -1)
		puts("[ERROR]socket");
	if (bind(server, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
		puts("[ERROR]bind");
	if (listen(server, LISTENQ) == -1)
		puts("[ERROR]listen");

	while (1)
	{
		clilen = sizeof(cliaddr);
		client = accept(server, (struct sockaddr *)&cliaddr, &clilen);
		if (client == -1)
			puts("[ERROR]accept");
		pid_t child = fork();
		if (!child)
		{
			echo(client);
			close(client);
			close(server);
			return 0;
		}
		close(client);
	}
}
