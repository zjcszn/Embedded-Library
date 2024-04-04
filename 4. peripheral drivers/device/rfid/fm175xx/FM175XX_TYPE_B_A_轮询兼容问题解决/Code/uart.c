/*************************************************************/
//2014.03.06�޸İ�
/*************************************************************/
//�����շ�����

#include "STC15F2K60S2.h"
#include "pin.h"
#include "uart.h"
#include <string.h>
#define UART_BAUD 115200	//115200bps
#define SYS_CLK 33177600 	//33.1776Mhz

struct status data uart_status;
unsigned char uart_buffer[255];
unsigned char data uart_point;		//����ָ��

/*********************************************/
/*��������	    timer_uart_init
/*���ܣ�	    ʱ���봮�ڳ�ʼ��
				��ʼ������0Ϊ1152000BPS
				1��ֹͣλ��8������λ����У��λ
				�����ж�����
/*���������	��
/*����ֵ��	    ��
/*********************************************/
void Timer_Uart_Init()
{
	SCON=0x50;			//SM0:0 SM1:1 SM2:0 REN:1 TB8:0 RB8:0 TI:0 RI:0
	PCON=0x00;			//SMOD=0  
	AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
	AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
	TMOD &= 0x0F;		//�趨��ʱ��1Ϊ16λ�Զ���װ��ʽ

	TL1=(65536-((long)SYS_CLK/4/(long)UART_BAUD));	  
	TH1=(65536-((long)SYS_CLK/4/(long)UART_BAUD))>>8;

	ET1 = 0;		//��ֹ��ʱ��1�ж�
	TR1 = 1;		//������ʱ��1

	INT1=1;			//�����жϽ�Ϊ�ߵ�ƽ
	IT1=1;			//�������½���
	EX1=0;			//�ж�ʹ��

	ES=1;	 		//������0�ж�
	EA=1;			//�����ж�
	return;
}

/********************************/
/*��������	    uart_int
/*���ܣ�	    �����жϳ���
/*���������	��
/*����ֵ��	    ��
/*******************************/
void Uart_Int() interrupt 4
{
	if(RI==1)                                           		//���ڽ����ж�
		{	
			if (SBUF != '#')
			{
				uart_buffer[uart_point]=SBUF;
				uart_point++;
			}
			else
			{
				uart_status.rece_finish=1;
			}
			RI=0;
		}

	if(TI==1)                                           		//���ڷ����ж�
		{
			TI=0;
			uart_status.send_finish	=1;
		}
	return;
}

//void Timer0_Int(void) interrupt 1
//{
// 	TH0=timer_H;
//	TL0=timer_L;
//	return;
//}

void Uart_Send_Msg(unsigned char *msg)
{ 
	unsigned int data i;
	unsigned int data msg_len;
	msg_len= strlen(msg);
//	REN=0;
 	for (i=0;i<msg_len;i++)
	{
		SBUF=msg[i];//
		while(uart_status.send_finish==0);
			uart_status.send_finish=0;
	}
//	REN=1;
	return;
}

void Uart_Rece_Msg(unsigned char recr_len)
{
	uart_point=0;
	while(uart_point!=recr_len);
		uart_point=0;
	return;
}

void Uart_Send_Hex(unsigned char *input_byte,unsigned int input_len)
{
unsigned char data temp_byte;
unsigned int data i;

	for(i=0;i<input_len;i++)
	{
		temp_byte=(*(input_byte+i) & 0xF0)>>4;
		if (temp_byte<0x0A)
			SBUF=0x30+temp_byte;
		else
			SBUF=0x37+temp_byte;

		while(uart_status.send_finish==0);
			uart_status.send_finish	=0;
	
		temp_byte=(*(input_byte+i) & 0x0F);
		if (temp_byte<0x0A)
			SBUF=0x30+temp_byte;
		else
		    SBUF=0x37+temp_byte;
		while(uart_status.send_finish==0);
			uart_status.send_finish	=0;
	}
	return;
}


//void Hex_to_BCD(unsigned long hex_input,unsigned char *BCD_output)
//{
//		BCD_output[0] = hex_input / (long)100000;
//		hex_input = hex_input - (BCD_output[0]* (long)100000);
//		BCD_output[1] = hex_input / (long)10000;
//		hex_input = hex_input - (BCD_output[1]* (long)10000);
//		BCD_output[2] = hex_input / (long)1000;
//		hex_input = hex_input -(BCD_output[2] *(long) 1000);
//		BCD_output[3] = hex_input / 100;
//		hex_input = hex_input - (BCD_output[3] * 100);
//		BCD_output[4] = hex_input / 10;
//		hex_input = hex_input - (BCD_output[4] * 10);
//		BCD_output[5] = hex_input / 1;
//		return;
//}