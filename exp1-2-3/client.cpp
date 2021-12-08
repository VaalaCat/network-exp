// client.cpp
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
#define MAXLINE 1024

int recvn(int s, char *recvbuf, unsigned int fixedlen)
{
	int n;	 //存储单次recv操作的返回值
	int cnt; //用于统计相对于固定长度，剩余多少字节尚未接收
	cnt = fixedlen;
	while (cnt > 0)
	{
		n = recv(s, recvbuf, cnt, 0);
		if (n < 0)
		{
			//数据接收出现错误
			printf("[ERROR]Recv Error\n");
			return -1;
		}
		if (n == 0)
		{
			//对方关闭连接
			printf("[INFO]Recv Finish\n");
			return fixedlen - cnt;
		}
		recvbuf += n;
		//更新cnt值
		cnt -= n;
	}
	return fixedlen;
}

int recvvl(int s, char *buf, unsigned int recvbuflen)
{
	int n;			  //存储单次recv操作的返回值
	unsigned int Len; //获取接收报文长度信息
	n = recvn(s, (char *)&Len, sizeof(unsigned int));
	if (n != sizeof(unsigned int))
	{
		printf("[ERROR]Recv Error\n");
		return -1;
	}
	Len = ntohl(Len);

	if (Len > recvbuflen)
	{
		while (Len > 0)
		{
			n = recvn(s, buf, recvbuflen);
			if (n != recvbuflen)
			{
				if (n == -1)
				{
					printf("[ERROR]Recv Error\n");
					return -1;
				}
				else
				{
					printf("[INFO]Loss Connected\n");
					return 0;
				}
			}

			Len -= recvbuflen;
			if (Len < recvbuflen)
				recvbuflen = Len;
		}
		printf("[ERROR]Message Too Long\n");
		return -1;
	}

	//接收可变长消息
	n = recvn(s, buf, Len);
	if (n != Len)
	{
		if (n == -1)
		{
			printf("[ERROR]Recv Error\n");
			return -1;
		}
		else
		{
			printf("[INFO]Loss Connected\n");
			return 0;
		}
	}
	return n;
}

void Interactive(int sd)
{
	unsigned int sendDataLen = 0;
	unsigned int buflen = MAXLINE;
	char sendBuf[MAXLINE];
	char recvBuf[MAXLINE];
	int retVal = 0;

	while (1)
	{
		memset(recvBuf, 0, MAXLINE);
		read(0, sendBuf, MAXLINE);

		if (strcmp(sendBuf, "q") == 0)
		{
			printf("Byebye\n");
			return;
		}

		sendDataLen = (unsigned int)strlen(sendBuf);
		sendDataLen = htonl(sendDataLen);
		if (send(sd, (char *)&sendDataLen, sizeof(unsigned int), 0) <= 0)
		{
			printf("[ERROR]Send Len error.\n");
			return;
		}
		if (send(sd, sendBuf, strlen(sendBuf), 0) <= 0)
		{
			printf("[ERROR]Send Massage Error.\n");
			return;
		}
		retVal = recvvl(sd, recvBuf, buflen);
		if (retVal == -1)
		{
			printf("[ERROR]Recvvl Error\n");
			return;
		}
		printf("%s\n", recvBuf);
	}
}

int main()
{
	int sock_client;
	sockaddr_in Servaddr;

	memset(&Servaddr, 0, sizeof(Servaddr));

	//创建套接字
	if ((sock_client = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("[ERROR]Creat socket Failed\n");
		exit(1);
	}

	Servaddr.sin_family = AF_INET;
	Servaddr.sin_port = htons(SERVER_PORT);
	Servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock_client, (struct sockaddr *)&Servaddr, sizeof(Servaddr)))
	{
		printf("[ERROR]Connect Failed\n");
		return -1;
	}

	printf("[INFO]Connect Succeed!\n");
	Interactive(sock_client);
	close(sock_client);
	return 0;
}
