// client.cpp
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //设置地址
	serv_addr.sin_port = htons(1234);					//设置端口
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
	{ // 连接服务器
		close(sock);
		puts("[ERROR]CONNECT FAILED!");
		exit(-1);
	}

	char buffer[40];
	read(sock, buffer, sizeof(buffer) - 1); //接收数据

	printf("Time: %s\n", buffer); //输出数据

	close(sock);
	return 0;
}
