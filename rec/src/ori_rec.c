#include <math.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "rec.h"
#include "crc32.h"

#define R_LEN 50
#define NAME_LEN 20

#define MAX_PACK 65536


unsigned char path_name[300];
//uint16_t end_len;
char *fdir(unsigned char *fn)
{
    strcpy(path_name, "/home/lilac/ftp/rec_file/");
    strcat(path_name, fn);
    return path_name;
}

void _rw_state(transfer_state_t *fail_excel, int flag)
{
    FILE *fp = fopen(STATE_FILE, "r+");/////
    //printf("%d",*fp);
    if(fp == NULL){
        printf("error");
    }

    if(flag == 1){
        fwrite((void*)fail_excel, sizeof(fail_excel), 1, fp);

    }else{
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        if(size == 0){
            memset(fail_excel, 0, sizeof(transfer_state_t));
        }else{
            fread((void*)fail_excel, sizeof(transfer_state_t), 1, fp);
        }
    }
    fclose(fp);
}

static unsigned char _check_crc(unsigned char *data, int len)
{
    unsigned char crc;
    unsigned char num = 0;
    for (int i = 0; i < len; i++){
        num = num + data[i];
    }
    crc = ~(num % 256);
    return crc;
}
// To be done
// data_len should be checked
// function indicating the end of a reception
void rec_new_packet(unsigned char *data, int data_len, transfer_state_t *state)
{
   // packet_state_t start_pack;
    packet_data_t d_block;
   // unsigned char file_name;
    uint32_t pack_f_number;
    uint32_t pack_f_size;
    uint32_t fail_seq[FAIL_NUM];
    unsigned char read_data[BLOCK_N];
    uint16_t file_len;

    extern uint32_t file_crc;

    if (_check_crc(data, data_len))
        return;

    memcpy((void*)&pack_f_number, (void*)data + 1, 4);

    if (data[0] == 0xaa){
        // received "start trans"
        printf("0xaa:");
        for(int i = 0; i < data_len; i++){
            printf("%x,", data[i]);
        }
        printf("\n");
        if(pack_f_number != state->f_number){                 //init state

            memcpy((void*)&(pack_f_size), (void*)(data + 6 + data[5]), 4);

            memcpy((void*)&(state->f_number), (void*)(data + 1), 4);
            memcpy((void*)&(state->f_name), (void*)(data + 6), data[5]);
            memcpy((void*)&(state->f_crc), (void*)(data + 10 + data[5]), 4);
            memset((void*)&(state->fail_buf), 1, sizeof(state->fail_buf));
            memset((void*)&(state->b_order), 0, 1);
            memcpy((void*)&(state->b_len), (void*)(data + 14 + data[5]), 2);

            state->rec_state = 0xfffffffe;
           // d_block.b_seq = 0;
            if(pack_f_size % state->b_len == 0){
                state->b_num = pack_f_size / (state->b_len);
                state->end_len = state->b_len;
            }else{
                state->b_num = pack_f_size / (state->b_len) + 1;
                state->end_len = pack_f_size - ((state->b_num - 1)*(state->b_len));
            }
        }
       // _rw_state(state, 1);
        printf("head: 0xaa:\n");
        printf("f_size: ");
        printf("%x\n", pack_f_size);
        printf("b_len: ");
        printf("%x\n", state->b_len);
        printf("b_num: ");
        printf("%d\n", state->b_num);
        printf("crc: %08x\n", state->f_crc);
    }

    else if (data[0] == 0xbb)
    {

        if(pack_f_number == state->f_number){

            memcpy((void*)&(d_block.b_seq), (void*)(data + 5), 4);
            memcpy((void*)&(d_block.b_len), (void*)(data + 9), 2);
            memcpy((void*)&(d_block.block), (void*)(data + 11), d_block.b_len);

            state->b_order = d_block.b_seq;
            //FILE *fp = fopen("~/Documents/transfile_test/trans_start.f_name", "wb+");/////
            FILE *fp = fopen(fdir(state->f_name),"r+");
            if(fp == NULL){
                fp = fopen(fdir(state->f_name),"w+");
                fclose(fp);
                fp = fopen(fdir(state->f_name),"r+");
               // printf("file created\n");
            }

            fseek(fp, ((d_block.b_seq)*(state->b_len)), 0);
            fwrite(d_block.block, d_block.b_len, 1, fp);///
            fclose(fp);
            state->fail_buf[d_block.b_seq] = 0;
            _rw_state(state, 1);
        }
    }

    else if (data[0] == 0xcc)                                                      // received "request trans state"
    {

        unsigned char trans_data[10 + 4*FAIL_NUM];
        uint32_t trans_len;

        trans_data[0] = 0xdd;

        if(pack_f_number == state->f_number)
        {
            memcpy((void*)(trans_data + 1), (void*)&state->f_number, 4);

            int err_sum = 0;
            for(int i = 0; i < state->b_num; i++){
                err_sum = err_sum + state->fail_buf[i];
            }

            if (err_sum == state->b_num){
                state->rec_state = 0xfffffffe;
                memcpy((void*)(trans_data + 5), (void*)&(state->rec_state), 4);
                trans_len = 10;
            }else if (err_sum == 0){

                FILE *fp = fopen(fdir(state->f_name),"r+");
                if(fp == NULL){
                    printf("Open File Error!\n");
                }else{
                    for(int i = 0; i < state->b_num; i++){
                        fseek(fp, i*(state->b_len), 0);
                        if(i < (state->b_num - 1)){
                            file_len = state->b_len;
                        }else{
                            file_len = state->end_len;
                            printf("end_len = %d", state->end_len);
                        }
                        fread(read_data, 1, file_len, fp);
                        file_crc = crc32_calc(read_data, file_len, file_crc);
                    }
                }
                fclose(fp);
                printf("cal crc = %08x\n", file_crc);
                if(file_crc == state->f_crc ){
                    state->rec_state = 0x00000000;
                    printf("Receive Finished!");
                    memcpy((void*)(trans_data + 5), (void*)&(state->rec_state), 4);
                    trans_len = 10;
                }else{
                    state->rec_state = 0xffffffff;
                    printf("CRC ERROR! ");
                    memcpy((void*)(trans_data + 5), (void*)&(state->rec_state), 4);
                    trans_len = 10;
                }

            }else{
                int j = 0;
                for(int i = 0; i < state->b_num; i++ ){
                    if(state->fail_buf[i] == 1){
                        fail_seq[j] = i;
                        j++;
                    }
                    if (j > FAIL_NUM -1){
                        printf("j = %d", j);
                        break;
                    }
                }
                memcpy((void*)(trans_data + 5), (void*)&j, 4);
                memcpy((void*)(trans_data + 9), (void*)&fail_seq, 4*j);
                trans_len = 10 + 4*j;
            }
            ///send
            trans_data[trans_len - 1] = _check_crc(trans_data, trans_len - 1);
            _rw_state(state, 1);

            rec_send(trans_data, trans_len);

            printf("trans_len = %d\n", trans_len);
            printf("send: ");
            for(int i = 0; i < trans_len; i++){
                printf("%x, ", trans_data[i]);
            }
            printf("\n");
        }
    }
}

void rec_init(transfer_state_t *state)
{
    _rw_state(state, 0);
}

void rec_send(unsigned char *data, int len)
{
    extern int sockfd,new_fd;
    send(new_fd, data, len, 0);
}
