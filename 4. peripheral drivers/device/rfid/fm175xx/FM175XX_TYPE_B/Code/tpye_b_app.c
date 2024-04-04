
#include "uart.h"
#include "FM175XX.h"
#include "main.h"
#include "type_b.h"
#include <string.h>

unsigned char TYPE_B_APP(void)
{
unsigned char rece_len;
unsigned char rece_buff[64];
	Set_Rf(0);   //�ر�TX1��TX2���
	Set_Rf(3);   //ѡ��TX1��TX2���
	if(TypeB_Request(&rece_len,rece_buff,&PICC_B)==ERROR)
		return ERROR;
	Uart_Send_Msg("-> TYPE B Request\r\n");
  Uart_Send_Msg("-> ATQB =");
	Uart_Send_Hex(rece_buff,rece_len);
	Uart_Send_Msg("\r\n");

	if(TypeB_Select(PICC_B.PUPI,&rece_len,rece_buff)==ERROR)
		return ERROR;
   	Uart_Send_Msg("-> TYPE B AttriB\r\n");
   	Uart_Send_Msg("-> Return =");
	Uart_Send_Hex(rece_buff,rece_len);
	Uart_Send_Msg("\r\n");

	//

//   	if(TypeB_Halt(PICC_B.PUPI,&rece_len,rece_buff)==ERROR)
//		return ERROR;
//   	Uart_Send_Msg("-> TYPE B Halt\r\n");
//   	Uart_Send_Msg("-> Return =");
//	Uart_Send_Hex(rece_buff,rece_len);
//	Uart_Send_Msg("\r\n");




	return OK;

}