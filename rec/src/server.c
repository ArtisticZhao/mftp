#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include"rec.h"

#define PORT 1500//�˿ں�
#define BACKLOG 5/*��������*/

#define MAX_DATA 500//���յ����������̶�
char buf[MAX_DATA];//�����������
int sockfd,new_fd;

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
    unsigned char data[500];
    uint16_t len;

    rec_init(&state);

    while(1)
    {

        usleep(10000);
        len = recv(new_fd, data, MAX_DATA, 0);//���������ݴ���buf�������ֱ��Ǿ�������洦����󳤶ȣ�������Ϣ����Ϊ0���ɣ���
        rec_new_packet(data, len, &state);
    }

    return 0;
}
