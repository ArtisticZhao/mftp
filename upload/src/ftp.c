#include "ftp.h"
#include "crc32.h"
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
//#include <errno.h>
#define PERIOD 100


unsigned char path_name[300];
//memset(path_name, 0, 300);
char *fdir(char *fn)
{
	//char *path_name = (char*)malloc(300);
	//strcpy(path_name, "/home/bg2dgr/Downloads/");
	//strcat(path_name, fn);
	//printf("%s\n", path_name);
	return path_name;
}

void ftp_packet_send(unsigned char *data, int len)
{

}

int file_size(unsigned char *filename, uint32_t *crc)
{
	unsigned char data[DATA_LEN];
    uint32_t size;
	uint16_t len;
	uint16_t N;

	*crc = 0xFFFFFFFF;

	//char name;
	FILE *fp = fopen(fdir(filename),"r");

	if(fp == NULL ){
		printf("\nerror on open file!");
		exit(1);
	}
	else{
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);

        if(size%DATA_LEN == 0)
            N = size/DATA_LEN;
        else
            N = size/DATA_LEN + 1;

        for(int i = 0; i < N; i++){

            if (size%DATA_LEN != 0 && i == N - 1){
                len = size%DATA_LEN;
            }else
                len = DATA_LEN;

            fseek(fp, i*DATA_LEN, 0);
            fread(data, 1, len, fp);

            *crc = crc32_calc(data, len, *crc);
        }

        fclose(fp);

	}
	return size;
}

int data_precode(unsigned char *ori_data, int ori_len, unsigned char *return_data)
{

    int len = 1;

	return_data[0] = 0xc0;
	for (int i = 0; i < ori_len; i++){
        if(ori_data[i] == 0xc0){
            return_data[len] = 0xdb;
            len++;
            return_data[len] = 0xdc ;
            len++;
        }else if(ori_data[i] == 0xdb ){
            return_data[len] = 0xdb;
            len++;
            return_data[len] = 0xdd ;
            len++;
        }else{
            return_data[len] = ori_data[i];
            len++;
        }
	}
	return_data[len] = 0xc0;
	len++;

	return len;

}

int file_read(pkg_start_t *tx, int N, int file_order, unsigned char *b_data)
{
	int last_size;
	int b_size;

    unsigned char buffer[DATA_LEN];

    memset(buffer, '\0', sizeof(buffer));

	last_size = tx->f_size - (N-1)*DATA_LEN;

	if(file_order < N-1){
        b_size = DATA_LEN;
	}else{
        b_size = last_size;
	}

	FILE *fp = fopen(fdir(tx->f_name),"r");
	fseek(fp, file_order * tx->b_len, 0);
	fread(buffer, b_size, 1, fp);
	fclose(fp);

    //

	//printf("send data: %d \n", file_order);
	/*for(int i = 0; i < b_size; i++) {
            printf("%c", buffer[i]);
	}
	printf("\n");*/
   // printf("b_size: %x \n", b_size);

	b_data[0] = 0xbb;
	memcpy((void*)(b_data + 1), (void*)&(tx->f_number), 4);
	memcpy((void*)(b_data + 5), (void*)&file_order, 4);
	memcpy((void*)(b_data + 9), (void*)&b_size, 2);
	memcpy((void*)(b_data + 11), (void*)buffer, b_size);
	b_data[11+b_size] = packet_crc_calc(b_data, 11 + b_size);
	b_size = b_size + 12;
	return b_size;
}

void upload_packet_send(int dst, unsigned char *data, int len)
{

     extern int sockfd,new_fd;/*cocket句柄和接受到连接后的句柄 */
     send(sockfd, data,len,0);

}

void upload_proc(int dst, unsigned char *file_name, uint8_t filename_len, uint32_t f_number)
{
	unsigned char buf[MAX_DATA];

	pkg_start_t tx_start;
	pkg_state_t trans_state;

	unsigned char frame_tx[30];
	unsigned char r_frame_tx[30*2];
	int frame_len;
	unsigned char ori_request_state[6];
	unsigned char request_state[12];
	int state_len;
	unsigned char data[12+DATA_LEN];
	unsigned char data_buf[DATA_LEN*2];
    int len = 0;
    extern int sockfd,new_fd;

    uint32_t ready_state;

    int data_size;
    int s_data_size;

    uint32_t loss_N;
    uint32_t loss_OR;

    memset(buf,'\0',sizeof(buf));

    for(int i = 0; i <MAX_DATA; i++ ){
        buf[i] = 0;
    }

//file NO.
    memset(&tx_start,0,sizeof(tx_start));
	tx_start.head = 0xaa;
	tx_start.f_number = f_number;
	tx_start.name_len = filename_len;
	memcpy((void*)tx_start.f_name, (void*)file_name, filename_len);
	tx_start.f_size = file_size(file_name, &(tx_start.f_crc));
	tx_start.b_len = DATA_LEN;

    memcpy((void*)frame_tx, (void*)&(tx_start.head), 1);
	memcpy((void*)(frame_tx + 1), (void*)&(tx_start.f_number), 4);
	memcpy((void*)(frame_tx + 5), (void*)&(tx_start.name_len), 1);
	memcpy((void*)(frame_tx + 6), (void*)&(tx_start.f_name), tx_start.name_len);
	memcpy((void*)(frame_tx + 6 + tx_start.name_len), (void*)&(tx_start.f_size), 4);
	memcpy((void*)(frame_tx + 10 + tx_start.name_len), (void*)&(tx_start.f_crc), 4);
	memcpy((void*)(frame_tx + 14 + tx_start.name_len), (void*)&(tx_start.b_len), 2);
	uint8_t tx_len = tx_start.name_len + 16 + 1;
    frame_tx[tx_len - 1] = packet_crc_calc(frame_tx, tx_len-1);

    frame_len = data_precode(frame_tx, tx_len, r_frame_tx);

//request transmit state
	ori_request_state[0] = 0xcc;
	memcpy((void*)(ori_request_state + 1), (void*)&(tx_start.f_number), 4);
	ori_request_state[5] = packet_crc_calc(ori_request_state, 5);
	state_len = data_precode(ori_request_state, 6, request_state);

/////
	//FILE *fd = fopen(_PATH_NAME_, "wb");
	while(1)
	{
	   // START_TRANS:
		////write(can_socket,frame_tx,filename_len+16+1);
		upload_packet_send(dst, r_frame_tx, frame_len);
		printf("f_size: %x \n", tx_start.f_size);
		usleep(10000);
		////write(can_socket,request_state,sizeof(request_state));
		upload_packet_send(dst, request_state, state_len);

		//sleep(1);

		len = recv(sockfd, buf, MAX_DATA, 0);

        printf("0xaa:");
		for(int i = 0; i < frame_len; i++){
            printf("%x, ", r_frame_tx[i]);
		}
		printf("\n");

	    if (len<=0){
            printf("recall state!\n");
            usleep(10000);
            continue;
		}
	/*	printf("Received: ");
		for (int i = 0; i < len; i++ ){
            printf("%x, ", buf[i]);
        }
        printf("\n");*/

		if(packet_crc_calc(buf, len) == 0){
           // trans_state.head = buf[0];
           if(buf[0] == 0xdd){
                memcpy((void*)&trans_state.f_number,(void*)(buf + 1), 4);
                memcpy((void*)&trans_state.rec_state,(void*)(buf + 5), 4);
                if(trans_state.f_number == tx_start.f_number){
                    if(trans_state.rec_state == 0xfffffffe){
                        break;
                    }else if(trans_state.rec_state == 0xffffffff){
                        continue;
                    }else{
                        memcpy((void*)&trans_state.fail_seq, (void*)(buf + 9), trans_state.rec_state);
                        break;
                    }
                }
           }
		}
	}
//           ready state

	int N;
	if(tx_start.f_size%DATA_LEN == 0)
		N = tx_start.f_size/DATA_LEN;
	else
		N = tx_start.f_size/DATA_LEN + 1;
    printf("block_num: %d \n", N);

	//unsigned char loss_data[FAIL_NUM];
    int i = 0;
	//for(int i = 0; i < N; i++){
    while(1){

            if(buf[0] = 0xdd){
                memcpy((void*)&ready_state,(void*)(buf + 5), 4);
                if(ready_state == 0x00000000){
                    printf("send finished!\n");
                    break;
                }else if(ready_state == 0xffffffff){
                    printf("error");
                    break;
                }else{
                    if(ready_state == 0xfffffffe){
                        loss_N = PERIOD;
                    }else
                        loss_N = ready_state;
                    for(int j = 0; j < loss_N; j++){
                        if(ready_state == 0xfffffffe)
                            loss_OR = j;
                        else
                            memcpy((void*)&loss_OR, (void*)(buf+9+j*4), 4);

                      //  printf("LOSS_N = %d", loss_N);

                        data_size = file_read(&tx_start, N, loss_OR, data);//loss number
                        s_data_size = data_precode(data, data_size, data_buf);
                        upload_packet_send(dst, data_buf, s_data_size);
                        i++;
                        usleep(10000);
                    ////write(can_socket,&loss_data,sizeof(loss_data));
                    }

                    while(1){

                        usleep(10000);

                        printf("------>>>>>>ask state\n");

                        upload_packet_send(dst, request_state, state_len);
                        printf("request_state: ");
                        for(int i = 0; i<6; i++){
                            printf("%x,", request_state[i]);
                        }
                        printf("\n");

                        len = recv(sockfd, buf, MAX_DATA, 0);

                       // printf("len = %d\n", len);
                        if(len <= 0){
                            printf("Timeout!!\n");
                        }else if (packet_crc_calc(buf, len) == 0){

                            printf("rec_file:\n");
                            for (int i = 0; i < len; i++ ){
                                printf("%x, ", buf[i]);
                            }
                            printf("\n");
                            break;
                        }
                    }
            }
        }
    }
}



