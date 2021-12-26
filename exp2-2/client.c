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