# 实验二 基于数据包套接字的网络程序设计

## 实验环境

```
arm64-apple-darwin21.2.0
Apple clang version 13.0.0 (clang-1300.0.29.30)
```

## 实验内容

### 设计思路

1. 基于数据报套接字的服务器回射程序设计 

	编写一服务器程序和客户程序，如图1，要求客户每输入一行数据，服 务器接收后加上echo:回送给客户程序，当客户输入“q”后退出。 
 
分别以循环服务器和并发服务器实现。 

2. 无连接应用程序丢包率测试 

	UDP的不可靠性使得基于该协议的应用程序在数据通信过程中不可避 免地会遇到丢包现象。一方面，网络拥塞导致路由器转发数据报文时丢失; 另一方面，慢速设备来不及处理快速到达的数据报文，使得接收缓存溢出而 丢包，等等。在应用程序开发前，设计者需要对当前的网络状况和主机性能 进行测试，以确定选择哪种协议承载运输、使用循环方式还是并发方式处理 网络通信等等，其中丢包率测试是常用的项目，它可以辅助设计者对程序的 可靠性进行直观的探测和诊断。  
	使用数据报套接字编程，在网络功能框架的基础上对回射服务器和客户端进行修改，实现丢包率测试工具。其中，服务器能够接收客户端发来的数据，统计数据报个数;客户端能够根据用户的指示向服务器批量发送数据。 丢包率的计算公式如下: 
	丢包率=1-(服务器收到的报文个数/客户端发送的报文个数)×100% 
	通过套接字使用类似于Ping的思路来计算RTT来反映延迟的大小，可以多次发包根据多次的结果来计算一个平均的RTT；丢包率可以通过指定发送端发包的数量，然后在接收端统计接收成功的数量，就可以计算出当前丢包率。

### 程序清单

#### 基于数据包套接字的服务器回射程序设计

##### 循环服务器

服务端程序

```cpp
//server.c
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
```

客户端程序

```cpp
//client.c：
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

#define SERV_PORT 1234
#define MAXLINE 1024

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    int n;
    char recvBuf[MAXLINE+1], sendBuf[MAXLINE];

    while(fgets(sendBuf, MAXLINE, stdin) != NULL)
    {
        sendto(sockfd, sendBuf, strlen(sendBuf), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        n = recvfrom(sockfd, recvBuf, MAXLINE, 0, NULL, NULL);
        recvBuf[n] = '\0';
        fputs(recvBuf, stdout);
        if (!strcmp(sendBuf, "q\n"))
            break;
    }
}
```

##### 并发服务器

客户端程序

```cpp
//client.c
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

#define SERV_PORT 1234
#define MAXLINE 1024

int main(int argc, char **argv)
{
  int sockfd;
  struct sockaddr_in servaddr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERV_PORT);
  inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

  int n;
  char recvBuf[MAXLINE + 1], sendBuf[MAXLINE];

  int flag = 1;
  while (fgets(sendBuf, MAXLINE, stdin) != NULL)
  {
    if (flag)
    {
      sendto(sockfd, sendBuf, strlen(sendBuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
      n = recvfrom(sockfd, recvBuf, MAXLINE, 0, NULL, NULL);
      flag = 0;

      int port = atoi(recvBuf);

      bzero(&servaddr, sizeof(servaddr));
      servaddr.sin_family = AF_INET;
      servaddr.sin_port = htons(port);
      inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    }
    else
    {
      sendto(sockfd, sendBuf, strlen(sendBuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    }
    
    n = recvfrom(sockfd, recvBuf, MAXLINE, 0, NULL, NULL);
    recvBuf[n] = '\0';
    fputs(recvBuf, stdout);
    if (!strcmp(sendBuf, "q\n"))
      break;
  }
}
```

服务端程序

```cpp
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
```

#### 无连接程序丢包率测试

客户端程序

```cpp
//client.c
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
#include <time.h>
#include <math.h>

#define CLOCKS_PER_SEC 1000000

void udp_delay_detect_client(const char* ip,int port,int packetSize,int packetNum)
{
    int sock = socket(PF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr(ip);
    servAddr.sin_port = htons(port);

    struct sockaddr_in clntAddr;
    int nSize = sizeof(servAddr);
    char bufSend[packetSize+1];
    memset(bufSend,'a',sizeof(bufSend));
    bufSend[packetSize]='\0';

    char bufRecv[packetSize+1];
    sendto(sock,bufSend,strlen(bufSend),0,(struct sockaddr*)&servAddr, sizeof(servAddr));       

    int send_count=packetNum;
    int recv_count=0;

    double sum_delay=0;
    double sum_jitter=0;
    double last_delay;

    int recv_miss_count=0;

    int timeout = 2000;
    int ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));

    while(send_count--){
        sendto(sock, bufSend, strlen(bufSend), 0, (struct sockaddr*)&servAddr, sizeof(servAddr));
        clock_t start=clock();
        int strLen = recvfrom(sock, bufRecv, packetSize, 0, &clntAddr, &nSize);
        if(strLen<1)
        {
            continue;
        }
        clock_t end=clock();
        recv_count++;
        double delay=(double)(end-start)*1000 / CLOCKS_PER_SEC;
        double jitter;
        if(send_count==packetNum){jitter=0,last_delay=delay;continue;}
        jitter=fabs(delay-last_delay);
        sum_delay+=delay;
        sum_jitter+=jitter;
    }

    double delay=sum_delay/packetNum;
    double jitter=sum_jitter/packetNum;
    printf("RRT: %lf ms jitter: %lf ms packet_loss_rate:%lf\n", delay, jitter, 1-(double)recv_count/packetNum);
    close(sock);
}

int main()
{
    udp_delay_detect_client("127.0.0.1", 9999, 0x100, 10);
    return 0;
}
```

服务端程序

```cpp
//server.c
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
#include <time.h>
#define BUF_SIZE 0x1000

void udp_delay_detect_server(int port)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(port);
    bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr));

    struct sockaddr_in clntAddr;
    int nSize = sizeof(clntAddr);
    char buffer[BUF_SIZE];
    
    recvfrom(sock,buffer,BUF_SIZE,0,&clntAddr,&nSize);
    
    int recv_count=0;
 
    int timeout = 2000;
    int ret=setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout));

    int recv_miss_count=0;
    while(1){
        int strLen = recvfrom(sock, buffer, BUF_SIZE, 0, &clntAddr, &nSize);
        if(strLen>1)              
        {
            sendto(sock, buffer, strLen, 0, &clntAddr, nSize);
            recv_count++;
        }
        else{
            recv_miss_count++;
            puts("no packet recieve!");
            if(recv_miss_count==3)          
                break;
        }
    }
    printf("packet_recv_count:%d\n", recv_count);
    close(sock);
}

int main()
{
    udp_delay_detect_server(9999);
    return 0;
}
```

### 用户使用说明（输入 / 输出规定）

1. 基于数据包套接字的服务器回射程序设计

	1. 循环服务器

		服务器端 `./server`

		客户端 `./client IP`

	2. 并发服务器

		服务器端 `./server`

		客户端 `./client IP`

2. 无连接应用程序丢包率测试

服务器端 `./server`

客户端 `./client [包数]`

### 运行结果（要求截图）

#### 基于数据包套接字的服务器回射程序设计

1. 循环服务器

客户端截图
		
![](./2021-12-15-19-12-03.png)

服务器截图
	
![](./2021-12-15-19-11-36.png)

2. 并发服务器

Server：

![](./2021-12-15-19-15-36.png)

Client1:

![](./2021-12-15-19-15-59.png)
		
Client2:

![](./2021-12-15-19-16-17.png)

#### 无连接应用程序丢包率测试

Server：

![](./2021-12-15-19-22-34.png)
	
Client：

![](./2021-12-15-19-22-53.png)

输入发送数据包的次数，自动退出。

修改服务器缓冲区大小，得到不同结果：

|超时时间/ms	|客户端发包/个|	丢包率/%|
|--|--|--|
|2000|	10|	0|
|2000|	20|	0|
|2000|	30|	0|
|2000|	40|	0|
|2000|	50|	0|
|2000|	60|	0|
|2000|	70|	0|
|2000|	80|	0|
|2000|	90|	0|
|2000|	100	|0|

