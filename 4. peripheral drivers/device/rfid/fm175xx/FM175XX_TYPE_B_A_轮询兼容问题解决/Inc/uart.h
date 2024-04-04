#ifndef UART_H
#define UART_H

struct status
{
 unsigned char send_finish;		//������ɷ���һ֡���ݱ�־λ
 unsigned char rece_finish;		//������ɽ���һ֡���ݱ�־λ
 unsigned char rece_error;			//���ڽ��ճ����־
 unsigned char send_error;			//���ڷ��ͳ����־
};
extern struct status data uart_status;

extern unsigned char uart_buffer[255];	//���ڻ�����

extern unsigned char data uart_point;		//����ָ��


extern void Timer_Uart_Init();
extern void Uart_Send_Msg(unsigned char *msg);
extern void Uart_Rece_Msg(unsigned char recr_len);
extern void Uart_Send_Hex(unsigned char *input_byte,unsigned int input_len);
//extern void Hex_to_BCD(unsigned long hex_input,unsigned char *BCD_output);
#endif