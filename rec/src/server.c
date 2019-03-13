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

#define PORT 1500//�˿ں�
#define BACKLOG 5/*��������*/

#define MAX_DATA 500//���յ����������̶�
//char buf[MAX_DATA];//�����������
int sockfd,new_fd;
uint32_t file_crc;

int main()
{

    /*socket����ͽ������Ӻ�ľ��*/
    struct sockaddr_in my_addr;/*������ַ��Ϣ�ṹ�壬�����о�������Ը�ֵ*/
    struct sockaddr_in their_addr;/*�Է���ַ��Ϣ*/
    int sin_size;

    sockfd=socket(AF_INET,SOCK_STREAM,0);//����socket

    if(sockfd==-1)
    {
        printf("socket failed:%d",errno);
        return -1;
    }

    printf("socket created!\n");

    my_addr.sin_family=AF_INET;/*�����Ա�ʾ���ձ�����������������*/
    my_addr.sin_port=htons(PORT);/*�˿ں�*/
    my_addr.sin_addr.s_addr=htonl(INADDR_ANY);/*IP���������ݱ�ʾ����IP*/
    bzero(&(my_addr.sin_zero),8);/*������������0*/

    int reuse0 = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse0, sizeof(reuse0));

    if(bind(sockfd,(struct sockaddr*)&my_addr,sizeof(struct sockaddr))<0) //�󶨵�ַ�ṹ���socket
    {
        printf("bind error");
        return -1;
    }

    listen(sockfd,BACKLOG);//�������� ���ڶ�����������������

    sin_size=sizeof(struct sockaddr_in);
    new_fd=accept(sockfd,(struct sockaddr*)&their_addr,&sin_size);//����������֪�����յ���Ϣ�������ֱ���socket��������յ��ĵ�ַ��Ϣ�Լ���С
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

        len = recv(new_fd, data, MAX_DATA, 0);//���������ݴ���buf�������ֱ��Ǿ�������洦����󳤶ȣ�������Ϣ����Ϊ0���ɣ���

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







