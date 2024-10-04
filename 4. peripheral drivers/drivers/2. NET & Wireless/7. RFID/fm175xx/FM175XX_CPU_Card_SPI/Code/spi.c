/*************************************************************/
//2014.07.15�޸İ�
/*************************************************************/
#include "STC15F2K60S2.h"
#include "intrins.h"
#include "pin.h"
#include "spi.h"
#include "main.h"


unsigned char SPI_Init()
{
	SPDAT=0;
	SPSTAT=SPIF|WCOL;//���SPIF��WCOL��־
	SPCTL=0xD0;//SSIG=1,SPEN=1,DORD=0,MSTR=1,CPOL=0,CPHA=0,SPR1=0,SPR0=0 (SPI_CLK=CPU_CLK/4)
	AUXR1=0x00;//PCA_P4=0,SPI_P4=0(SPI��P1 Port),S2_P4=0,GF2=0,ADRJ=0,DPS=0
	return OK;
}
/*******************************************************************************************************/
/*���ƣ�																							   */
/*���ܣ�SPI�ӿڶ�ȡ����	SPI read function															   */
/*����:																								   */
/*		N/A																							   */
/*�����																							   */
/*		addr:	��ȡFM175XX�ڵļĴ�����ַ[0x00~0x3f]	reg_address									   */
/*		rddata:	��ȡ������							reg_data										   */
/*******************************************************************************************************/
unsigned char SPIRead(unsigned char addr)	//SPI������
{
 unsigned char data reg_value,send_data;
	SPI_CS=0;

   	send_data=(addr<<1)|0x80;
	SPSTAT=SPIF|WCOL;//���SPIF��WCOL��־
	SPDAT=send_data;
	while(!(SPSTAT&SPIF));

	SPSTAT=SPIF|WCOL;//���SPIF��WCOL��־
	SPDAT=0x00;
	while(!(SPSTAT&SPIF));
	reg_value=SPDAT;
	SPI_CS=1;
	return(reg_value);
}

void SPIRead_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value)	
//SPI����������,����READ FIFO����
{
 unsigned char data i,send_data;
 	if (sequence_length==0)
		return;

	SPI_CS=0;

   	send_data=(addr<<1)|0x80;
	SPSTAT=SPIF|WCOL;//���SPIF��WCOL��־
	SPDAT=send_data;
	while(!(SPSTAT&SPIF));

	for (i=0;i<sequence_length;i++)
	{
		SPSTAT=SPIF|WCOL;//���SPIF��WCOL��־
		if (i==sequence_length-1)
			SPDAT=0x00;//���һ�ζ�ȡʱ����ַ����0x00
		else
			SPDAT=send_data;

		while(!(SPSTAT&SPIF));
		*(reg_value+i)=SPDAT;
 	}
	SPI_CS=1;
	return;
}	
/*******************************************************************************************************/
/*���ƣ�SPIWrite																					   */
/*���ܣ�SPI�ӿ�д������	  SPI write function														   */
/*����:																								   */
/*		add:	д��FM17XX�ڵļĴ�����ַ[0x00~0x3f]	  reg_address									   */
/*		wrdata:   Ҫд�������						  reg_data										   */
/*�����																							   */
/*		N/A																							   */
/*******************************************************************************************************/
void SPIWrite(unsigned char addr,unsigned char wrdata)	//SPIд����
{
unsigned char data send_data;
	SPI_CS=0;				   

	send_data=(addr<<1)&0x7e;
	
  	SPSTAT=SPIF|WCOL;//���SPIF��WCOL��־
	SPDAT=send_data;
	while(!(SPSTAT&SPIF));

	SPSTAT=0xC0;
	SPDAT=wrdata;
	while(!(SPSTAT&SPIF));

	SPI_CS=1;
	return ;	
}

void SPIWrite_Sequence(unsigned char sequence_length,unsigned char addr,unsigned char *reg_value)
//SPI����д����,����WRITE FIFO����
{
unsigned char data send_data,i;
	if(sequence_length==0)
		return;

	SPI_CS=0;

	send_data=(addr<<1)&0x7e;
	
  	SPSTAT=SPIF|WCOL;//���SPIF��WCOL��־
	SPDAT=send_data;
	while(!(SPSTAT&SPIF));

	for (i=0;i<sequence_length;i++)
	{
		SPSTAT=0xC0;
		SPDAT=*(reg_value+i);
		while(!(SPSTAT&SPIF));
	}

	SPI_CS=1;
	return ;	
}