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