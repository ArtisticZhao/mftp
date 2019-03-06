#ifndef __REC_H__
#define __REC_H__

#define R_LEN 50
#define D_LEN 256
#define NAME_LEN 20
#define FAIL_NUM 10
#define BLOCK_N 256
#define STATE_FILE "/home/lilac/ftp/tmp/state"
#define _RX_PATH_ "/home/lilac/ftp/tmp/tx.tmp"
#define _TX_PATH_ "/home/lilac/ftp/tmp/rx.tmp"
#define MAX_PACK 65536

typedef struct packet_state_t
{
    uint8_t head;
    uint32_t f_number;
    uint8_t name_len;
    uint8_t f_name[NAME_LEN];
    uint32_t f_size;
    uint32_t f_crc;
    uint16_t b_len;
    uint8_t pack_crc;
} packet_state_t;

typedef struct packet_data_t
{
    uint8_t head;
    uint32_t f_number;
    uint32_t b_seq;
    uint16_t b_len;
    uint8_t block[BLOCK_N];
    uint8_t pack_crc;
} packet_data_t;


typedef struct transfer_state_t
{
    uint8_t fail_buf[MAX_PACK];
    uint8_t f_name[NAME_LEN];
    uint32_t f_number;
    uint32_t f_crc;
    uint8_t b_order;
    uint8_t b_num;
    uint32_t rec_state;
} transfer_state_t;

void rec_init(transfer_state_t *state);
void rec_send(unsigned char *data, int len);
void rec_new_packet(unsigned char *data, int data_len, transfer_state_t *state);

#end
