 #include "fm175xx.h"
 #include "cpu_card.h"
 #include "main.h"
 #include "uart.h"
 #include <string.h>
 struct CPU_CARD_STR CPU_CARD;
 #define CPU_Card_Debug 0 
 
/****************************************************************/
/*����: Rats 													*/
/*����: �ú���ʵ��ת��APDU�����ʽ����	CPU card reset			*/
/*����: param1 byte :PCB 									    */
/*		param2 byte bit8-bit5��FSDI, bit4-bit0 ��CID  			*/
/*		param2 ͨ��ȡֵΪ0x51����FSDIΪ64�ֽڣ�CID=0x01			*/						
/*���:															*/
/*		rece_len�����յ����ݳ���								*/
/*		buff�����յ�����ָ��									*/
/* OK: Ӧ����ȷ													*/
/* ERROR: Ӧ�����												*/
/****************************************************************/
unsigned char CPU_Rats(unsigned char param1,unsigned char param2,unsigned char *rece_len,unsigned char *rece_buff)
{
	unsigned char data result,send_byte[2],offset,temp;
	unsigned int data rece_bitlen;
	CPU_CARD.FWT = 5;//FWTĬ��ֵԼΪ5ms
	Pcd_SetTimer(CPU_CARD.FWT);
	send_byte[0] = 0xE0;//Start byte
	send_byte[1] = param2;//ͨ��Ĭ��ֵΪ0x51
	CPU_CARD.CID = param2 & 0x0f;
    CPU_CARD.PCB = param1;//ͨ��Ĭ��ֵΪ0x02

	result=Pcd_Comm(Transceive,send_byte,2,rece_buff,&rece_bitlen);
	*rece_len = rece_bitlen/8;
	if ((result == OK)&&(*rece_len == rece_buff[0]))
		{
		offset = 0;
		CPU_CARD.ATS.TL=*rece_buff;
			#if CPU_Card_Debug
			Uart_Send_Msg("<- TL = ");Uart_Send_Hex(&CPU_CARD.ATS.TL,1);Uart_Send_Msg("\r\n");
			#endif
		CPU_CARD.ATS.T0 = *(rece_buff+1);
		
		CPU_CARD.ATS.FSCI = CPU_CARD.ATS.T0&0x0F;
		CPU_CARD.FSD = CPU_FSD(CPU_CARD.ATS.FSCI);
			#if CPU_Card_Debug
			Uart_Send_Msg("<- T0 = ");Uart_Send_Hex(&CPU_CARD.ATS.T0,1);Uart_Send_Msg("\r\n");
			Uart_Send_Msg("<- CPU_CARD.FSD = ");Uart_Send_Hex(&CPU_CARD.FSD,1);Uart_Send_Msg("\r\n");
			#endif

		if(CPU_CARD.ATS.T0&0x40)
			{
			CPU_CARD.ATS.TA1 = *(rece_buff+2+offset);
			#if CPU_Card_Debug
			Uart_Send_Msg("<- TA1 = ");Uart_Send_Hex(&CPU_CARD.ATS.TA1,1);Uart_Send_Msg("\r\n");
			#endif
			offset++;
			}
			else
			{
			CPU_CARD.ATS.TA1 = 0;
			#if CPU_Card_Debug
			Uart_Send_Msg("<- TA1 is not transmitted\r\n");
			#endif
			}
		if(CPU_CARD.ATS.T0&0x20)
			{
			CPU_CARD.ATS.TB1 = *(rece_buff+2+offset);
			CPU_CARD.ATS.FWI = (CPU_CARD.ATS.TB1&0xF0)>>4;
			CPU_CARD.FWT = CPU_FWT(CPU_CARD.ATS.FWI);
			CPU_CARD.ATS.SFGI = CPU_CARD.ATS.TB1&0x0F;
			offset++;

			#if CPU_Card_Debug
			Uart_Send_Msg("<- TB1 = ");Uart_Send_Hex(&CPU_CARD.ATS.TB1,1);Uart_Send_Msg("\r\n");
			Uart_Send_Msg("<- CPU_CARD.ATS.FWI = ");Uart_Send_Hex(&CPU_CARD.ATS.FWI,1);Uart_Send_Msg("\r\n");
			temp = CPU_CARD.FWT>>8;
			Uart_Send_Msg("<- CPU_CARD.FWT H = ");Uart_Send_Hex(&temp,1);Uart_Send_Msg("\r\n");
			temp = CPU_CARD.FWT&0xFF;
			Uart_Send_Msg("<- CPU_CARD.FWT L = ");Uart_Send_Hex(&temp,1);Uart_Send_Msg("\r\n");
			#endif
			}
			else
			{
			CPU_CARD.ATS.TB1 = 0;
			#if CPU_Card_Debug
			Uart_Send_Msg("<- TB1 is not transmitted\r\n");
			#endif
			}
		if(CPU_CARD.ATS.T0&0x10)
			{
			CPU_CARD.ATS.TC1 = *(rece_buff+2+offset);
			#if CPU_Card_Debug
			Uart_Send_Msg("<- TC1 = ");Uart_Send_Hex(&CPU_CARD.ATS.TC1,1);Uart_Send_Msg("\r\n");
			#endif
			offset++;
			}
			else
			{
			CPU_CARD.ATS.TC1 = 0;
			#if CPU_Card_Debug
			Uart_Send_Msg("<- TC1 is not transmitted\r\n");
			#endif
			}
		}
	else
		result = ERROR;
	return result;	
}
/****************************************************************/
/*����: PPS 													*/
/*����: 														*/
/*����: PPSS byte, PPS0 byte, PPS1 byte							*/
/*���:															*/
/*		rece_len�����յ����ݳ���								*/
/*		buff�����յ�����ָ��									*/
/* OK: Ӧ����ȷ													*/
/* ERROR: Ӧ�����												*/
/****************************************************************/
unsigned char CPU_PPS(unsigned char pps0,unsigned char pps1,unsigned char *rece_len,unsigned char *rece_buff)
{
	unsigned char data result,send_byte[3];
	unsigned int data rece_bitlen;
	Pcd_SetTimer(CPU_CARD.FWT);
	send_byte[0] = 0xD0|CPU_CARD.CID;//ppss byte
	send_byte[1] = pps0;	//pps0 byte	
	//106K: pps0=0x11(pps1����),pps1=0x00(DSI:00,DRI:00)
	//212K: pps0=0x11(pps1����),pps1=0x05(DSI:01,DRI:01)
	//424K:	pps0=0x11(pps1����),pps1=0x0A(DSI:10,DRI:10)
	if ((pps0&0x10) == 0x10)
	{
		send_byte[2] = pps1; 	//pps1 byte
		result = Pcd_Comm(Transceive,send_byte,3,rece_buff,&rece_bitlen );
	}
	else
	{	
		result = Pcd_Comm(Transceive,send_byte,2,rece_buff,&rece_bitlen );
	}
	if (result == OK)
		*rece_len = rece_bitlen/8;
	else
		*rece_len = 0;
	return result;	
}

void CPU_PCB_CONVER(void)
{
	CPU_CARD.PCB = CPU_CARD.PCB^0x01;//���ÿ�ű任
return;
}

/****************************************************************/
/*����: CPU_command 	cpu card operation						*/
/*����: CPU�������												*/
/*����:															*/
/*																*/
/*		send_len:���͵�ָ��� 								*/
/*		send_buff:���͵�ָ������ָ��	 						*/
/*																*/
/*���:��Ӧ����													*/
/*		rece_len�����յ����ݳ���								*/
/*		rece_buff�����յ�����ָ��								*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
unsigned char CPU_TPDU(unsigned char send_len,unsigned char *send_buff,unsigned char *rece_len,unsigned char *rece_buff)
{	unsigned char data result;
	unsigned int data rece_bitlen;

	Pcd_SetTimer(CPU_CARD.FWT);
	result = Pcd_Comm(Transceive,send_buff,send_len,rece_buff,&rece_bitlen);
	if (result == OK)
			*rece_len = rece_bitlen/8;
		else
			*rece_len = 0;
	return result;
}
/****************************************************************/
/*����: I Block 	cpu card operation							*/
/*����: CPU�������(I block)									*/
/*����:															*/
/*																*/
/*		send_len:���͵�ָ��� 								*/
/*		send_buff:���͵�ָ������ָ��	 						*/
/*																*/
/*���:��Ӧ����													*/
/*		rece_len�����յ����ݳ���								*/
/*		rece_buff�����յ�����ָ��								*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
 unsigned char CPU_I_Block(unsigned char send_len,unsigned char *send_buff,unsigned char *rece_len,unsigned char *rece_buff)
 {
		unsigned char data result,i,tpdu_tx_len;
		unsigned int data rece_bitlen;

CPU_I_BLOCK_START:
		Pcd_SetTimer(CPU_CARD.FWT);
		if( CPU_CARD.PCB&0x08)//cid����
			 {
			 	memmove(send_buff+2,send_buff,send_len);
			 	memcpy(send_buff,&CPU_CARD.PCB,1);//pcb�ֽ�
			 	memcpy(send_buff+1,&CPU_CARD.CID,1);//cid�ֽ�
			 	tpdu_tx_len=send_len+2;
			 }
			 else
			 {
			 	memmove(send_buff+1,send_buff,send_len);
			 	memcpy(send_buff,&CPU_CARD.PCB,1);//pcb�ֽ�
			 	tpdu_tx_len=send_len+1;
			 }
		result = Pcd_Comm(Transceive,send_buff,tpdu_tx_len,rece_buff,&rece_bitlen);
 		
   		if (result == OK)
		{
			CPU_CARD.PCB = *rece_buff;
			if((CPU_CARD.PCB&0xF0) == 0xF0)//WTX
				{
				if(CPU_CARD.PCB&0x08)
					CPU_CARD.WTXM = *(rece_buff+2);
				else
					CPU_CARD.WTXM = *(rece_buff+1);
				}
				#if CPU_Card_Debug
		   			Uart_Send_Msg("<- I BLOCK CPU_CARD.PCB = ");Uart_Send_Hex(&CPU_CARD.PCB,1);Uart_Send_Msg("\r\n");
		   		#endif
			*rece_len = rece_bitlen/8;
			return OK;
		}
		else
		{
			for (i=0;i<3;i++)
				{
				   result = CPU_NAK(rece_len,rece_buff);
				   if (result == OK)
			 		{
						#if CPU_Card_Debug
		   					Uart_Send_Msg("-> I Block NAK OK\r\n");//
		   				#endif
					CPU_CARD.PCB = *rece_buff;

					if((CPU_CARD.PCB&0xF0) == 0xA0)//�յ�ACK
						{
							#if CPU_Card_Debug
		   						Uart_Send_Msg("-> I Block ACK\r\n");//
								Uart_Send_Msg("<- CPU_CARD.PCB = ");Uart_Send_Hex(&CPU_CARD.PCB,1);Uart_Send_Msg("\r\n");
		   					#endif

						CPU_CARD.PCB=CPU_CARD.PCB&0x0F;
						CPU_PCB_CONVER();
						goto CPU_I_BLOCK_START;
						}

					if((CPU_CARD.PCB&0xF0) == 0xF0)//�յ�WTX
						{
						if(CPU_CARD.PCB&0x08)
							CPU_CARD.WTXM = *(rece_buff+2);
						else
							CPU_CARD.WTXM = *(rece_buff+1);
						}
			   
				   	return OK;
				   }
				}
				#if CPU_Card_Debug
		   			Uart_Send_Msg("-> I Block NAK ERROR\r\n");//
		   		#endif
		}
		*rece_len=0;
		return ERROR;
 }
/****************************************************************/
/*����: R Block 	cpu card operation							*/
/*����: CPU�������(R block)									*/
/*����:															*/
/*																*/
/*																*/
/*���:��Ӧ����													*/
/*		rece_len�����յ����ݳ���								*/
/*		rece_buff�����յ�����ָ��								*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/

 unsigned char CPU_R_Block(unsigned char *rece_len,unsigned char *rece_buff)
 {
 	unsigned char data send_buff[2],result,i;
	unsigned int data rece_bitlen;

CPU_R_BLOCK_START:
	Pcd_SetTimer(CPU_CARD.FWT);

	if (CPU_CARD.PCB&0x08)//�ж��Ƿ���CID�ֽ�
		{
  		send_buff[0] = (CPU_CARD.PCB&0x0F)|0xA0;
		send_buff[1] = CPU_CARD.CID;
		result=Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
		}
	else
		{
		send_buff[0] = (CPU_CARD.PCB&0x0F)|0xA0;
		result=Pcd_Comm(Transceive,send_buff,1,rece_buff,&rece_bitlen);
		}
	
	if (result == OK)
		{
			CPU_CARD.PCB = *rece_buff;
			if((CPU_CARD.PCB&0xF0) == 0xA0)//�յ�ACK
						{
							#if CPU_Card_Debug
		   						Uart_Send_Msg("-> R Block ACK\r\n");//
								Uart_Send_Msg("<- CPU_CARD.PCB = ");Uart_Send_Hex(&CPU_CARD.PCB,1);Uart_Send_Msg("\r\n");
		   					#endif
						CPU_CARD.PCB=CPU_CARD.PCB&0x0F;
						CPU_PCB_CONVER();
						goto CPU_R_BLOCK_START;
						}

			if((CPU_CARD.PCB&0xF0) == 0xF0)//�յ�WTX
				{
				if(CPU_CARD.PCB&0x08)
					CPU_CARD.WTXM = *(rece_buff+2);
				else
					CPU_CARD.WTXM = *(rece_buff+1);
				}
			CPU_PCB_CONVER();
			*rece_len = rece_bitlen/8;
			return OK;
		}
	else
		{
		
			for (i = 0 ;i<3;i++)
				{
				   result = CPU_NAK(rece_len,rece_buff);
				   if (result == OK)
			 		{
						#if CPU_Card_Debug
		   					Uart_Send_Msg("-> R Block NAK OK\r\n");//
		   				#endif
					CPU_CARD.PCB=*rece_buff;
					if((CPU_CARD.PCB&0xF0) == 0xF0)//WTX
						{
						if(CPU_CARD.PCB&0x08)
							CPU_CARD.WTXM = *(rece_buff+2);
						else
							CPU_CARD.WTXM = *(rece_buff+1);
						}
					CPU_PCB_CONVER();
					*rece_len = rece_bitlen/(unsigned int)8;
				   	return OK;
				   }
				}
				#if CPU_Card_Debug
		   			Uart_Send_Msg("-> R Block NAK ERROR\r\n");//
		   		#endif

		}
			*rece_len = 0;
	return ERROR;
 }
/****************************************************************/
/*����: S Block(WTX) 	cpu card operation						*/
/*����: CPU�������(S block,WTX)								*/
/*����:															*/
/*																*/
/*		pcb_byte:����PCB�ֽ�									*/
/*																*/
/*���:��Ӧ����													*/
/*		rece_len�����յ����ݳ���								*/
/*		rece_buff�����յ�����ָ��								*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
 unsigned char CPU_WTX(unsigned char wtxm,unsigned char *rece_len,unsigned char *rece_buff)
 {
   unsigned char data send_buff[3],result;
   unsigned int data rece_bitlen;
   Pcd_SetTimer(CPU_CARD.FWT);
   send_buff[0] = CPU_CARD.PCB;
    if (send_buff[0]&0x08)//�ж��Ƿ���CID�ֽ�
	   {
		  	send_buff[1] = CPU_CARD.CID;
			send_buff[2] = wtxm;
			result=Pcd_Comm(Transceive,send_buff,3,rece_buff,&rece_bitlen);
		}
	else
		{
		  	send_buff[1] = wtxm;
			result = Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
		}
	if (result == OK)
		*rece_len = rece_bitlen/8;
	else
		*rece_len = 0;
	return result;
 }

/****************************************************************/
/*����: CPU_baud_rate_change									*/
/*����: ����ͨ�Ų�����											*/
/*����:															*/
/*		mode=0 ���շ��Ͳ�����106kbps							*/
/*		mode=1 ���շ��Ͳ�����212kbps							*/
/*		mode=2 ���շ��Ͳ�����424kbps	 						*/
/*																*/
/*���:��Ӧ����													*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
unsigned char CPU_BaudRate_Set(unsigned char mode) //mode=0:106Kbps;1:212Kpbs;2:424Kpbs
{
	Clear_BitMask(TxModeReg,0x70);
	Clear_BitMask(RxModeReg,0x70);
  switch (mode)
  {
  case 0:	  //106kbps
 		Set_BitMask(TxModeReg,0x00);
		Set_BitMask(RxModeReg,0x00);
        Write_Reg(ModWidthReg,0x26);                
		break;
  case 1:	  //212kbps
		Set_BitMask(TxModeReg,0x10);
		Set_BitMask(RxModeReg,0x10);
        Write_Reg(ModWidthReg,0x13);
		break;
  case 2:	  //424kbps
		Set_BitMask(TxModeReg,0x20);
		Set_BitMask(RxModeReg,0x20);
        Write_Reg(ModWidthReg,0x09);
		break;
  case 3:	  //848kbps
		Set_BitMask(TxModeReg,0x30);
		Set_BitMask(RxModeReg,0x30);
        Write_Reg(ModWidthReg,0x04);
		break;
  }
return OK;
}
/****************************************************************/
/*����: S Block(Deselect) 	cpu card operation					*/
/*����: CPU�������(S block,Deselect)							*/
/*����:															*/
/*																*/
/*		pcb_byte:����PCB�ֽ�									*/
/*																*/
/*���:��Ӧ����													*/
/* 		OK: Ӧ����ȷ											*/
/* 		ERROR: Ӧ�����											*/
/****************************************************************/
unsigned char CPU_Deselect(void)
{
	unsigned char data result,rece_len,send_buff[2],rece_buff[2];
	unsigned int data rece_bitlen;
	Pcd_SetTimer(CPU_CARD.FWT);
	CPU_CARD.PCB=0xC2|(CPU_CARD.PCB&0x08);//PCB=CA����PCB=C2��ΪDeselect����
	send_buff[0] = CPU_CARD.PCB;
 	if(CPU_CARD.PCB&0x08)
		{
		send_buff[1] = CPU_CARD.CID;
		result = Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
		}
	else
		{
		result = Pcd_Comm(Transceive,send_buff,1,rece_buff,&rece_bitlen);
		}

	if((result == OK)&&(rece_buff[0] == CPU_CARD.PCB))
		return OK;
	else
		return ERROR;	
}

unsigned char CPU_FSD(unsigned char fsdi)
{
unsigned char data fsd;
	 switch (fsdi&0x0F)
	  {
	  case 0:	 
	 		fsd = 14;//Э�鶨��Ϊ16����Ҫ��ȥ2���ֽ�CRC������Ϊ14
			break;
	  case 1:	  
			fsd = 22;
			break;
	  case 2:	  
			fsd = 30;
			break;
	  case 3:	  
			fsd = 38;
			break;
	  case 4:	  
			fsd = 46;
			break;
	  case 5:	  
			fsd = 62;
			break;
	  case 6:	  
			fsd = 94;
			break;
	  case 7:	  
			fsd = 126;
			break;
	  case 8:	  
			fsd = 254;
			break;
	  default:
			fsd = 256;
			break;
	  }
return fsd;
}

unsigned int CPU_FWT(unsigned char fwi)
{
unsigned int data fwt;
	 switch (fwi&0x0F)
	  {
	  case 0:	 
	 		fwt = 1;//Э�鶨��Ϊ302us����������Ϊ1ms�����ڶ�ʱ����������СֵΪ1ms��
			break;
	  case 1:	  
			fwt = 1;//604us
			break;
	  case 2:	  
			fwt = 1;//1208us
			break;
	  case 3:	  
			fwt = 3;//2416us
			break;
	  case 4:	  
			fwt = 5;//4833us
			break;
	  case 5:	  
			fwt = 10;//9666us
			break;
	  case 6:	  
			fwt = 20;//19332us
			break;
	  case 7:	  
			fwt = 40;//38664us
			break;
	  case 8:	  
			fwt = 80;//77328us
			break;
	  case 9:	  
			fwt = 155;//154657us
			break;
	  case 10:	  
			fwt = 310;//309314us
			break;
	  case 11:	  
			fwt = 619;//618628us
			break;
	  case 12:	  
			fwt = 1238;//1237257us
			break;
	  case 13:	  
			fwt = 2475;//2474515us
			break;
	  case 14:	  
			fwt = 4950;//4949031us
			break;
 	  default:
			fwt = 5;
			break;
	  }
return fwt;

}


 unsigned char CPU_NAK(unsigned char *rece_len,unsigned char *rece_buff)
 {
 	unsigned char data send_buff[2],result;
	unsigned int data rece_bitlen;
	Pcd_SetTimer(CPU_CARD.FWT);

	if (CPU_CARD.PCB&0x08)//�ж��Ƿ���CID�ֽ�
		{
  		send_buff[0] = (CPU_CARD.PCB&0x0F)|0xB0;
		send_buff[1] = CPU_CARD.CID;
		result=Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
		}
	else
		{
		send_buff[0] = (CPU_CARD.PCB&0x0F)|0xB0;
		result = Pcd_Comm(Transceive,send_buff,1,rece_buff,&rece_bitlen);
		}
	if (result == OK)
		{
			CPU_CARD.PCB = *rece_buff;
			if((CPU_CARD.PCB&0xF0)==0xF0)//WTX
				{
				if(CPU_CARD.PCB&0x08)
					CPU_CARD.WTXM = *(rece_buff+2);
				else
					CPU_CARD.WTXM = *(rece_buff+1);
				}
		
			*rece_len = rece_bitlen/8;
		}
	else
			*rece_len = 0;
	return result;
 }

 unsigned char CPU_ACK(unsigned char *rece_len,unsigned char *rece_buff)
 {
 	unsigned char data send_buff[2],result;
	unsigned int data rece_bitlen;
	Pcd_SetTimer(CPU_CARD.FWT);

	if (CPU_CARD.PCB&0x08)//�ж��Ƿ���CID�ֽ�
		{
  		send_buff[0] = (CPU_CARD.PCB&0x0F)|0xA0;
		send_buff[1] = CPU_CARD.CID;
		result=Pcd_Comm(Transceive,send_buff,2,rece_buff,&rece_bitlen);
		}
	else
		{
		send_buff[0] = (CPU_CARD.PCB&0x0F)|0xA0;
		result=Pcd_Comm(Transceive,send_buff,1,rece_buff,&rece_bitlen);
		}
	if (result==OK)
		{
			CPU_CARD.PCB=*rece_buff;
			if((CPU_CARD.PCB&0xF0)==0xF0)//WTX
				{
				if(CPU_CARD.PCB&0x08)
					CPU_CARD.WTXM=*(rece_buff+2);
				else
					CPU_CARD.WTXM=*(rece_buff+1);
				}
			
			*rece_len=rece_bitlen/8;
		}
	else
			*rece_len=0;
	return result;
 }