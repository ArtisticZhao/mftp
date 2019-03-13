#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include"rec.h"
#include"crc32.h"

#define PORT 1500//端口号
#define BACKLOG 5/*最大监听数*/

#define MAX_DATA 500//接收到的数据最大程度
//char buf[MAX_DATA];//储存接收数据
int sockfd,new_fd;
uint32_t file_crc;

int main()
{

    /*socket句柄和建立连接后的句柄*/
    struct sockaddr_in my_addr;/*本方地址信息结构体，下面有具体的属性赋值*/
    struct sockaddr_in their_addr;/*对方地址信息*/
    int sin_size;

    sockfd=socket(AF_INET,SOCK_STREAM,0);//建立socket

    if(sockfd==-1)
    {
        printf("socket failed:%d",errno);
        return -1;
    }

    printf("socket created!\n");

    my_addr.sin_family=AF_INET;/*该属性表示接收本机或其他机器传输*/
    my_addr.sin_port=htons(PORT);/*端口号*/
    my_addr.sin_addr.s_addr=htonl(INADDR_ANY);/*IP，括号内容表示本机IP*/
    bzero(&(my_addr.sin_zero),8);/*将其他属性置0*/

    int reuse0 = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse0, sizeof(reuse0));

    if(bind(sockfd,(struct sockaddr*)&my_addr,sizeof(struct sockaddr))<0) //绑定地址结构体和socket
    {
        printf("bind error");
        return -1;
    }

    listen(sockfd,BACKLOG);//开启监听 ，第二个参数是最大监听数

    sin_size=sizeof(struct sockaddr_in);
    new_fd=accept(sockfd,(struct sockaddr*)&their_addr,&sin_size);//在这里阻塞知道接收到消息，参数分别是socket句柄，接收到的地址信息以及大小
    if(new_fd==-1)
    {
        printf("socket connection failed\n");
    }
    else
    {
        printf("socket connected\n");
    }

    transfer_state_t state;
    unsigned char data[MAX_DATA];
    unsigned char buffer[MAX_DATA];
    uint16_t len;

    memset(data, 0, MAX_DATA);
    memset(buffer, 0, MAX_DATA);

    rec_init(&state);

  /*  FILE *fp = fopen(fdir(state.f_name),"r+");
    if(fp == NULL)
    {
        fp = fopen(fdir(state.f_name),"w+");
        fclose(fp);
        fp = fopen(fdir(state.f_name),"r+");
       // printf("file created\n");
    }*/
    int data_len = 0;
    file_crc = 0xffffffff;

    while(1)
    {

        len = recv(new_fd, data, MAX_DATA, 0);//将接收数据打入buf，参数分别是句柄，储存处，最大长度，其他信息（设为0即可）。

       // int j = 0;
        for(int i = 0; i < len; i++ ){
            if(data[i] == 0xdb){
                if(data[i + 1] == 0xdc){
                    buffer[data_len] = 0xc0;
                    data_len++;
                    i++;
                }else if(data[i + 1] == 0xdd){
                    buffer[data_len] = 0xdb;
                    data_len++;
                    i++;
                }
            }else if(data[i] == 0xc0){
                rec_new_packet(buffer, data_len, &state);
                memset(buffer, 0, MAX_DATA);
                data_len = 0;


            }else{
                buffer[data_len] = data[i];
                data_len++;
            }

        }
        //rec_new_packet(data, len, &state);
    }
   // fclose(fp);

    return 0;
}







