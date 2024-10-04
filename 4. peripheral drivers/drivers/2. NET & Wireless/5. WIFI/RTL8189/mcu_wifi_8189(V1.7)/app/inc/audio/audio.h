#ifndef _AUDIO_H
#define _AUDIO_H

#include "api.h"

#define AUDIO_TYPE_PCM			0
#define AUDIO_TYPE_ADPCM		1
#define AUDIO_TYPE_MP3 			2
#define AUDIO_TYPE_UNKOWN 		3

#define MAX_BLANK_BYTES				256

#define DEFAULT_AUDIO_NET_PORT		4700

#define AUDIO_DMA_BUFF_SIZE 		8192

#define AUDIO_DAC_INPUT_BUFF_SIZE  	2048

#define AUDIO_ADC_BUFF_SIZE  		2048

struct AUDIO_DEC_INPUT
{
	uint8_t old_type;
	uint8_t type;
	uint8_t bits;
	uint8_t channels;
	unsigned int samplerate;
	uint8_t *data;
	uint32_t data_len;
	uint32_t buff_len;
};

//dec�������,ͬʱҲ��dac���뻺��
struct AUDIO_DEC_OUTPUT
{
	uint32_t len;
	uint32_t write_circle_cnt;
	uint32_t play_circle_cnt;
	uint32_t int_flag;
	uint32_t need_wait;
	uint16_t write_pos;
	uint16_t play_pos;
	wait_event_t dac_tx_waitq;
	uint8_t *data;

};

struct AUDIO_ADC
{
	uint8_t type;
	volatile uint8_t int_flag;
	volatile uint8_t pending_flag;
	unsigned int samplerate;
	struct adpcm_state stat;
	uint32_t len;
	uint8_t *data;
	uint8_t *block_data;
};

struct MP3_HEADER
{
	unsigned int sync: 11; //ͬ����Ϣ
	unsigned int version: 2; //�汾
	unsigned int layer: 2; //��
	unsigned int protection: 1; // CRCУ��
	unsigned int bitrate_index: 4; //λ��
	unsigned int sampling_frequency: 2; //����Ƶ��
	unsigned int padding: 1; //֡������
unsigned int private:
	1; //������
	unsigned int mode: 2; //����ģʽ
	unsigned int mode_extension: 2; //����ģʽ
	unsigned int copyright: 1; // ��Ȩ
	unsigned int original: 1; //ԭ���־
	unsigned int emphasis: 2; //ǿ��ģʽ
};



struct AUDIO_CFG
{
	uint8_t audio_dev_open;
	uint8_t volume; // 0 -100
	BOOL play_mp3;
	uint16_t audio_net_port;
	int audio_socket_num;
	struct AUDIO_DEC_INPUT *dec_input;
	struct AUDIO_DEC_OUTPUT *dec_output;
	struct WAVE_FORMAT_HEAD *pcm;
	struct AUDIO_ADC *adc;
};

extern struct AUDIO_CFG audio_cfg;

void push_to_play_buff(uint8_t *val, int size);
void audio_dev_init(void);
int audio_dev_open(void);
int audio_dev_close(void);
int handle_audio_stream(unsigned char *buff, int size);
void switch_audio_mode(void);
#endif
