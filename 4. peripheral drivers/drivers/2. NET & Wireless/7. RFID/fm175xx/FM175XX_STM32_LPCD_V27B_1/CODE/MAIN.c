#include "DEVICE_CFG.H"
#include "UART.h"
#include <string.h>
#include <stdio.h> 
#include "FM175XX.h"
#include "FM175XX_REG.h"

#include "LPCD_API.h"
#include "LPCD_CFG.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_map.h"
int main(void)
{	  
unsigned char reg_data;

	RCC_Config();			//ʱ������
	GPIO_Config();		//GPIO����
	
	USART3_Config();	//UART3����
	
	SPI_Config();			//SPI����
		
	EXTI_Config();  	//�ⲿ�ж�����
			
	UART_Com_Para_Init(&UART_Com3_Para);    //��ʼ��Uart1��ͨѶ����

	Uart_Send_Msg("FM17522,FM17550 LPCD DEMO\r\n");
	Uart_Send_Msg("Release Version 27_1 2017.06.16\r\n");
	
	LED_ARM_WORKING_ON;
	
	FM175XX_HardReset();

	GetReg(JREG_VERSION,&reg_data);
	if(reg_data == 0x88)
		Uart_Send_Msg("IC Version = V03\r\n");
	if(reg_data == 0x89)
		Uart_Send_Msg("IC Version = V03+\r\n");
	Uart_Send_Msg("Lpcd Debug Info Enable,Y or N?\r\n");
	
	if(Scan_User_Reply() == True)
		Lpcd_Debug_Info = 1;
	else
		Lpcd_Debug_Info = 0;
	
	Uart_Send_Msg("Lpcd Debug Test Enable,Y or N?\r\n");
	
	if(Scan_User_Reply() == True)
		Lpcd_Debug_Test = 1;
	else
		Lpcd_Debug_Test = 0;
	LED_ARM_WORKING_OFF;	

	if(Lpcd_Calibration_Event()== SUCCESS)//����FM175XXУ׼		
		Lpcd_Calibration_Backup();
	else
		{
			while(1);
			//�״�У׼ʧ�ܣ�������
		}
	Lpcd_Set_Mode(1);//����LPCDģʽ	
		
	while(1)
	{	
		PWR_EnterSTOPMode( PWR_Regulator_ON, PWR_STOPEntry_WFI); //MCU ����STOPģʽ		
		Lpcd_Set_Mode(0);//�˳�LPCD 
		Lpcd_Get_IRQ(&Lpcd.Irq);		
		Lpcd_IRQ_Event();
		Lpcd_Set_Mode(1);//����LPCDģʽ			
	}

}








