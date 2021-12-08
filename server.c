#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
	int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serv_addr.sin_port = htons(1234);
	bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	listen(serv_sock, 20); //监听端口

	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size = sizeof(clnt_addr);
	int clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size); //接受客户端链接

	//处理时间信息
	char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	time_t timep;
	struct tm *p;
	time(&timep);
	p = gmtime(&timep);
	char str[100];
	sprintf(str, "%d/%d/%d %s %d:%d:%d\n", (1900 + p->tm_year), (1 + p->tm_mon), p->tm_mday, wday[p->tm_wday], 8 + p->tm_hour, p->tm_min, p->tm_sec);
	write(clnt_sock, str, sizeof(str)); //发送时间信息

	close(clnt_sock);
	close(serv_sock);

	return 0;
}
