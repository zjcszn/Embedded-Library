#ifndef UART_H
#define UART_H
extern unsigned char data timer_L,timer_H;
//extern unsigned char xdata uart_buffer[255];	//���ڻ�����
extern unsigned char data uart_data_length;		//�������ݳ���
extern unsigned char data uart_verify;			//��������У��
extern unsigned char data uart_point;		//����ָ��
extern bit data uart_receive_finish_flag;		//������ɽ���һ֡��Ч���ݱ�־λ

extern bit data uart_send_finish_flag;			//������ɷ���һ֡���ݱ�־λ
extern bit data uart_receive_error_flag;			//���ڽ��ճ����־
extern bit data uart_send_error_flag;			//���ڷ��ͳ����־

extern void Timer_Uart_Init();
extern void Uart_Send_Msg(unsigned char *msg);
extern void Uart_Rece_Msg(unsigned char recr_len);
extern void Uart_Send_Hex(unsigned char *input_byte,unsigned int input_len);
extern void Hex_to_BCD(unsigned long hex_input,unsigned char *BCD_output);
#endif