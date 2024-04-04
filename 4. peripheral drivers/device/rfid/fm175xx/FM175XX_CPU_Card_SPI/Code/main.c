/*************************************************************/
//2014.07.10�޸İ�
/*************************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "pin.h"
#include "uart.h"
#include "spi.h"
#include "FM175XX.h"
#include "main.h"
#include "type_a.h"
#include "type_b.h"
#include "mifare_card.h"
#include "ultralight.h"
#include "des.h"
#include "cpu_card.h"
#include "cpu_app.h"
#include <string.h>
void main()
{
	Io_Init();
	Timer_Uart_Init();
	SPI_Init();
	Uart_Send_Msg("FM17520,FM17522,FM17550 CPU Card Reader\r\n");
	Uart_Send_Msg("Version 3.0 2015.4.18\r\n");
	if(FM175XX_HardReset()!=OK)
		{
		Uart_Send_Msg("-> FM175XX Reset ERROR\r\n");//FM175XX��λʧ��
		while(1);
		}
	Uart_Send_Msg("-> FM175XX Reset OK\r\n");//FM175XX��λ�ɹ�
	Pcd_ConfigISOType(0);
	Set_Rf(3);   //turn on radio
	while(1)
	{
	if (TypeA_CardActivate(PICC_ATQA,PICC_UID,PICC_SAK)==OK)
		{
		Uart_Send_Msg("---------------------\r\n");
		Uart_Send_Msg("-> Card Activate OK\r\n");
		Uart_Send_Msg("<- ATQA = ");Uart_Send_Hex(PICC_ATQA,2);Uart_Send_Msg("\r\n");
		Uart_Send_Msg("<- UID = ");Uart_Send_Hex(PICC_UID,4);Uart_Send_Msg("\r\n");
		Uart_Send_Msg("<- SAK = ");Uart_Send_Hex(PICC_SAK,1);Uart_Send_Msg("\r\n");
		if(CPU_APP()!=OK)
			{
			Set_Rf(0);
			Set_Rf(3);
			}
		}
   }
}

/*****************************/
/*�������ܣ�	�ܽų�ʼ������
/*���������	��
/*���������	��
/*����ֵ��		��
/*ռ����Դ��	��
/*****************************/
void Io_Init()
{
	P0=0xFF;
	P1=0xFF;
	P2=0xFF;
	P3=0xFF;
	NPD=1;
	LED_2=0;
	LED_3=0;
	return;
}
/*************************/
/*�������ܣ�	��ʱ����
/*���������	delay_time
/*���������	��
/*����ֵ��		��
/*ռ����Դ��	��
/*************************/
void Delay_100us(unsigned int delay_time)		//0.1ms*delay_time
{
	unsigned int data i;
	for(i=0;i<delay_time;i++)
	{
		_nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); 
		_nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
		_nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
		_nop_(); _nop_(); _nop_(); _nop_();	_nop_(); _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
		_nop_(); _nop_(); _nop_(); _nop_();	_nop_(); _nop_(); _nop_(); _nop_();	_nop_(); _nop_();
	}
}




