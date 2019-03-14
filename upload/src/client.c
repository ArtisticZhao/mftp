#include "ftp.h"
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
//#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/wait.h>

#define DEST_IP "127.0.0.1"/*Ŀ���ַIP��������Ϊ����*/
//#define MAX_DATA 500//���յ����������̶�

int sockfd,new_fd;/*cocket����ͽ��ܵ����Ӻ�ľ�� */
void load_parms(char **argv, int *port, char *path, int *f_num);  // ��ȡ���в���
void lib_entry(char *path, int port, int f_num);
void close_socketfd();
void main(int argc, char **argv)
{
    lib_entry("/home/bg2dgr/Downloads/test.doc", 1500, 10);
}

void lib_entry(char *path, int port, int f_num){

    extern unsigned char path_name[300];
    strcpy(path_name, path);
	uint32_t file_flag;
	unsigned char file_name[20];
	uint8_t len;
        memset(file_name,"\0",20);
    // ��ȡ���в���

    struct sockaddr_in dest_addr;/*Ŀ���ַ��Ϣ*/
    char buf[MAX_DATA];//�����������

    sockfd=socket(AF_INET,SOCK_STREAM,0);/*����socket*/

    if(sockfd==-1)
    {
        printf("socket failed:%d",errno);
    }

    //��������������������
    dest_addr.sin_family=AF_INET;
    dest_addr.sin_port=htons(port);
    dest_addr.sin_addr.s_addr=inet_addr(DEST_IP);
    bzero(&(dest_addr.sin_zero),8);

    struct timeval tv = {1, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));

    if(connect(sockfd,(struct sockaddr*)&dest_addr,sizeof(struct sockaddr))==-1) //���ӷ�������������Ŀ���ַ�ʹ�С
    {
        printf("connect failed:%d",errno);//ʧ��ʱ���Դ�ӡerrno
    }
    else
    {
        printf("connect success\n");
    }


	//printf("input file name:");
	//scanf("%s",file_name);
	if(path_name[0] == '/'){
        printf("%s\n", path_name);
        printf("%s\n", strrchr(path_name, '/')+1);
        strcpy(file_name, strrchr(path_name, '/'));
	}else{
        strcpy(file_name, path_name);
	}
	//printf("input file number:");
	//scanf("%d",&file_flag);
	len = strlen(file_name);
    int dst = 0;
	upload_proc(dst, file_name, len, file_flag);
}

void close_socketfd(){
    printf("exit");
    close(sockfd);
    sockfd = 0;
}
void load_parms(char **argv, int *port, char *path, int* f_num)
{
    /*
        �����в����ж����ļ�����, ��tcpserver �˿ں�
    */
    for (int i = 1; i < 7; i += 2)
    {
        if (strcmp(argv[i], "-f") == 0)
        {
            strcpy(path, argv[i + 1]);
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            *port = atoi(argv[i + 1]);
        }else if (strcmp(argv[i], "-n") == 0){
            *f_num = atoi(argv[i + 1]);
        }
    }
}
