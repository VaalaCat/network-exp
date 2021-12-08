// server.c
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

#define SERVER_PORT 4560
#define MAXLINE 1024 //接收缓冲区长度
#define LISTENQ 1024 //监听队列长度

int recvn(int s, char *buf, unsigned int fixedlen)
{
	int n;	 //存储单次recv操作的返回值
	int cnt; //用于统计相对于固定长度，剩余多少字节尚未接收
	cnt = fixedlen;
	while (cnt > 0)
	{
		n = recv(s, buf, cnt, 0);
		if (n < 0)
		{
			printf("[ERROR] Recv Error\n");
			return -1;
		}
		if (n == 0)
		{
			printf("[INFO] Loss Connect.\n");
			return fixedlen - cnt;
		}
		buf += n;
		cnt -= n;
	}
	return fixedlen;
}

int recvvl(int s, char *buf, unsigned int bufLen)
{
	int n;			  //存储单次recv操作的返回值
	unsigned int Len; //用于存储报文头部存储的长度信息,获取接收报文长度信息

	n = recvn(s, (char *)&Len, sizeof(unsigned int));
	if (n != sizeof(unsigned int))
	{
		printf("[ERROR]Recv Error\n");
		return -1;
	}

	Len = ntohl(Len);
	if (Len > bufLen)
	{
		//如果没有足够的空间存储变长消息，返回错误
		while (Len > 0)
		{
			n = recvn(s, buf, bufLen);
			if (n != bufLen)
			{
				if (n == -1)
				{
					printf("[ERROR] Recv Error\n");
					return -1;
				}
				else
				{
					printf("[INFO] Loss Connect.\n");
					return 0;
				}
			}
			Len -= bufLen;
			if (Len < bufLen)
				bufLen = Len;
		}
		printf("[ERROR]Message Too Long\n");
		return -1;
	}

	n = recvn(s, buf, Len);
	if (n != Len)
	{
		if (n == -1)
		{
			printf("[ERROR] Recv Error\n");
			return -1;
		}
		else
		{
			printf("[INFO] Loss Connect.\n");
			return 0;
		}
	}
	return n;
}

void echo(int s)
{
	int recvlen = 0;
	char recvBuf[MAXLINE]; //接收缓存
	char sendBuf[MAXLINE]; //发送缓存
	unsigned int Len = 0;  //发送数据长度
	unsigned int bufLen = MAXLINE;
	while (1)
	{
		memset(recvBuf, 0, MAXLINE);
		memset(sendBuf, 0, MAXLINE);
		recvlen = recvvl(s, recvBuf, bufLen);
		if (recvlen == -1)
		{
			printf("[ERROR] Recv Error.\n");
			close(s);
			return;
		}
		sprintf(sendBuf, "Echo: %s", recvBuf);
		printf("%s\n", sendBuf);

		Len = (unsigned int)strlen(sendBuf);
		Len = htonl(Len);
		recvlen = send(s, (char *)&Len, sizeof(unsigned int), 0);
		if (recvlen == -1)
		{
			printf("[ERROR] Send Len Error.\n");
			close(s);
			return;
		}
		recvlen = send(s, sendBuf, strlen(sendBuf), 0);
		if (recvlen == -1)
		{
			printf("[ERROR] Send Buf Error.\n");
			close(s);
			return;
		}
	}
}

int main()
{
	int sock_server, sock_client;  //服务器监听套接字和连接套接字
	socklen_t iClilen = 0;		   //连接客户端数
	sockaddr_in Cliaddr, Servaddr; //通信地址

	sock_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock_server)
	{
		printf("[ERROR] Socket Failed!\n");
		return -1;
	}

	memset(&Servaddr, 0, sizeof(Servaddr));
	memset(&Cliaddr, 0, sizeof(Cliaddr));

	//服务器监听套接字地址
	Servaddr.sin_family = AF_INET;
	Servaddr.sin_port = htons(SERVER_PORT);
	Servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (bind(sock_server, (struct sockaddr *)&Servaddr, sizeof(Servaddr)) < 0)
	{
		printf("[ERROR] Bind Error.\n");
		close(sock_server);
		return -1;
	}

	if (listen(sock_server, LISTENQ) < 0)
	{
		printf("[EEROR] Listen Error.\n");
		close(sock_server);
		return -1;
	}

	while (1)
	{
		iClilen = sizeof(Cliaddr);
		sock_client = accept(sock_server, (struct sockaddr *)&Cliaddr, &iClilen);
		if (sock_client < 0)
		{
			printf("[ERROR]Connect Error.\n");
		}
		else
		{
			printf("New Connect : %d\n", sock_client);
			echo(sock_client);
		}
	}

	close(sock_server);
	return 0;
}
