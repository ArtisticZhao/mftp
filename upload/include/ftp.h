#ifndef __UPLOAD_H__
#define __UPLOAD_H__

#include <stdint.h>
#define DATA_LEN 256
#define _NAME_LEN_ 20
#define FAIL_NUM 10

extern unsigned char path_name[300];

typedef struct pkg_start_t
{
	uint8_t head;
	uint32_t f_number;
	uint8_t name_len;
	uint8_t f_name[_NAME_LEN_];
	uint32_t f_size;
	uint32_t f_crc;
	uint16_t b_len;
	uint8_t pack_crc;
} pkg_start_t;

typedef struct pkg_state_t
{
    uint8_t head;
    uint32_t f_number;
    uint32_t rec_state;
    uint32_t fail_seq[FAIL_NUM];
    uint8_t pack_crc;
} pkg_state_t;

void upload_proc(int dst, unsigned char* file_name, uint8_t filename_len, uint32_t f_number);
void upload_packet_send(int dst, unsigned char *data, int len);

#endif // __UPLOAD_H__
