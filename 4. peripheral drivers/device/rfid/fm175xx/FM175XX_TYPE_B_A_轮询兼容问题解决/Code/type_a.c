/*************************************************************/
//2014.03.06�޸İ�
/*************************************************************/
#include "main.h"
#include "FM175XX.h"
#include "type_a.h"
#include "uart.h"
unsigned char PICC_ATQA[2],PICC_UID[15],PICC_SAK[3];
#define Type_A_Debug 0  
/****************************************************************************************/
/*���ƣ�TypeA_Request																	*/
/*���ܣ�TypeA_Request��ƬѰ��															*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_Request(unsigned char *pTagType)
{
	unsigned char result,send_buff[1],rece_buff[2];
	unsigned int rece_bitlen;  
	Clear_BitMask(TxModeReg,0x80);//�ر�TX CRC
	Clear_BitMask(RxModeReg,0x80);//�ر�RX CRC
	Clear_BitMask(Status2Reg,0x08);//���MFCrypto1On
	Write_Reg(BitFramingReg,0x07);
	send_buff[0] =0x26;
   	Pcd_SetTimer(1);
	result = Pcd_Comm(Transceive,send_buff,1,rece_buff,&rece_bitlen);
	
	if ((result == OK) && (rece_bitlen == 2*8))
	{   
	 
		*pTagType     = rece_buff[0];
		*(pTagType+1) = rece_buff[1];
		return OK;
	}

	return ERROR;
}
/****************************************************************************************/
/*���ƣ�TypeA_WakeUp																	*/
/*���ܣ�TypeA_WakeUp��ƬѰ��															*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pTagType[0],pTagType[1] =ATQA                                         		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_WakeUp(unsigned char *pTagType)
{
	unsigned char result,send_buff[1],rece_buff[2];
	unsigned int rece_bitlen;  
	Clear_BitMask(TxModeReg,0x80);//�ر�TX CRC
	Clear_BitMask(RxModeReg,0x80);//�ر�RX CRC
	Clear_BitMask(Status2Reg,0x08);//���MFCrypto1On
	Write_Reg(BitFramingReg,0x07);
 	send_buff[0] = 0x52;
   	Pcd_SetTimer(1);
	result = Pcd_Comm(Transceive,send_buff,1,rece_buff,&rece_bitlen);

	if ((result == OK) && (rece_bitlen == 2*8))
	{   
		*pTagType     = rece_buff[0];
		*(pTagType+1) = rece_buff[1];
		return OK;
	}
 	return ERROR;
}
/****************************************************************************************/
/*���ƣ�TypeA_Anticollision																*/
/*���ܣ�TypeA_Anticollision��Ƭ����ͻ													*/
/*���룺selcode =0x93��0x95��0x97														*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID                            		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_Anticollision(unsigned char selcode,unsigned char *pSnr)
{
    unsigned char result,i,send_buff[2],rece_buff[5];
    unsigned int rece_bitlen;
	Clear_BitMask(TxModeReg,0x80);
	Clear_BitMask(RxModeReg,0x80);
	Clear_BitMask(Status2Reg,0x08);//���MFCrypto1On
    Write_Reg(BitFramingReg,0x00);
    Write_Reg(CollReg,0x80);
 
    send_buff[0] = selcode;
    send_buff[1] = 0x20;
    Pcd_SetTimer(1);
    result = Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);

    if ((result == OK)&&(rece_bitlen==5*8))
    {
    	 for (i=0; i<5; i++)
         {   
             *(pSnr+i)  = rece_buff[i];
         }
         if (pSnr[4] == (pSnr[0]^pSnr[1]^pSnr[2]^pSnr[3]))
		 	{
				return OK;
			}
  
    } 
  return ERROR;
}
/****************************************************************************************/
/*���ƣ�TypeA_Select																	*/
/*���ܣ�TypeA_Select��Ƭѡ��															*/
/*���룺selcode =0x93��0x95��0x97														*/
/*      pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 			    			     	*/
/*	       								 												*/
/*�����																			 	*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_Select(unsigned char selcode,unsigned char *pSnr,unsigned char *pSak)
{
    unsigned char result,i,send_buff[7],rece_buff[5];
    unsigned int rece_bitlen;
	Write_Reg(BitFramingReg,0x00);
  	Set_BitMask(TxModeReg,0x80);
	Set_BitMask(RxModeReg,0x80);
    Clear_BitMask(Status2Reg,0x08);//���MFCrypto1On
	
	send_buff[0] = selcode;
    send_buff[1] = 0x70;
    
    for (i=0; i<5; i++)
    {
    	send_buff[i+2] = *(pSnr+i);
    }
   	
	Pcd_SetTimer(1);
    result = Pcd_Comm(Transceive,send_buff,7,rece_buff,&rece_bitlen);
    
    if (result == OK)
    	{ 
		*pSak=rece_buff[0];
		return OK;
	 	}
    return ERROR;
}
/****************************************************************************************/
/*���ƣ�TypeA_Halt																		*/
/*���ܣ�TypeA_Halt��Ƭֹͣ																*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����																			 	*/
/*	       											                            		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_Halt(void)
{
    unsigned char result,send_buff[2],rece_buff[1],len;
	unsigned int rece_bitlen=0;
    send_buff[0] = 0x50;
    send_buff[1] = 0x00;
   	
   	Write_Reg(BitFramingReg,0x00);
  	Set_BitMask(TxModeReg,0x80);
	Set_BitMask(RxModeReg,0x80);

	Pcd_SetTimer(1);
    result=Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
	if(result==TIMEOUT_Err)	
    	return OK;//1ms����Ӧ��halt�ɹ�
	else
		#if Type_A_Debug
			Uart_Send_Msg("<- result = ");Uart_Send_Hex(&result,1);Uart_Send_Msg("\r\n");
			len=rece_bitlen>>8;
			Uart_Send_Msg("<- rece_bitlen H = ");Uart_Send_Hex(&len,1);Uart_Send_Msg("\r\n");
			len=rece_bitlen&0xFF;
			Uart_Send_Msg("<- rece_bitlen L = ");Uart_Send_Hex(&len,1);Uart_Send_Msg("\r\n");
		#endif
		return ERROR;

}
/****************************************************************************************/
/*���ƣ�TypeA_CardActivate																*/
/*���ܣ�TypeA_CardActivate��Ƭ����														*/
/*���룺																				*/
/*       			    			     												*/
/*	       								 												*/
/*�����	pTagType[0],pTagType[1] =ATQA 											 	*/
/*	       	pSnr[0],pSnr[1],pSnr[2],pSnr[3]pSnr[4] =UID 		                   		*/
/*	       	pSak[0],pSak[1],pSak[2] =SAK			                            		*/					
/*       	OK: Ӧ����ȷ                                                              	*/
/*	 		ERROR: Ӧ�����																*/
/****************************************************************************************/
unsigned char TypeA_CardActivate(unsigned char *pTagType,unsigned char *pSnr,unsigned char *pSak)
{
unsigned char result;
	
		result=TypeA_Request(pTagType);//Ѱ�� Standard	 send request command Standard mode
		
		if (result==ERROR)
		{
			return ERROR;
		}

			if 	((pTagType[0]&0xC0)==0x00)
				{
				result=TypeA_Anticollision(0x93,pSnr);
				if (result==ERROR)
					{
						return ERROR;
					}
				result=TypeA_Select(0x93,pSnr,pSak);
				if (result==ERROR)
					{
						return ERROR;
					}	
				}

			if 	((pTagType[0]&0xC0)==0x40)
				{
				result=TypeA_Anticollision(0x93,pSnr);
				if (result==ERROR)
					{
						return ERROR;
					}
				result=TypeA_Select(0x93,pSnr,pSak);
				if (result==ERROR)
					{
						return ERROR;
					}
				result=TypeA_Anticollision(0x95,pSnr+5);
				if (result==ERROR)
					{
						return ERROR;
					}
				result=TypeA_Select(0x95,pSnr+5,pSak+1);
				if (result==ERROR)
					{
						return ERROR;
					}
			   	}
			if 	((pTagType[0]&0xC0)==0x80)
				{
				result=TypeA_Anticollision(0x93,pSnr);
				if (result==ERROR)
					{
						return ERROR;
					}
				result=TypeA_Select(0x93,pSnr,pSak);
				if (result==ERROR)
					{
						return ERROR;
					}
				result=TypeA_Anticollision(0x95,pSnr+5);
				if (result==ERROR)
					{
						return ERROR;
					}
				result=TypeA_Select(0x95,pSnr+5,pSak+1);
				if (result==ERROR)
					{
						return ERROR;
					}
				result=TypeA_Anticollision(0x97,pSnr+10);
				if (result==ERROR)
					{
						return ERROR;
					}
				result=TypeA_Select(0x97,pSnr+10,pSak+2);
				if (result==ERROR)
					{
						return ERROR;
					}
				}
		return result;
}
