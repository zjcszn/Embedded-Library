#include "drivers.h"
#include "app.h"
#include "api.h"


#define SCCB_SID_H()	GPIO_SET(SIO_D_PORT_GROUP, SIO_D_PIN)
#define SCCB_SID_L()	GPIO_CLR(SIO_D_PORT_GROUP, SIO_D_PIN)

#define SCCB_SIC_H()	GPIO_SET(SIO_C_PORT_GROUP, SIO_C_PIN)
#define SCCB_SIC_L()	GPIO_CLR(SIO_C_PORT_GROUP, SIO_C_PIN)

#define SCCB_SID_IN		SIO_D_IN
#define SCCB_SID_OUT		SIO_D_OUT
#define SCCB_SID_STATE	GPIO_STAT(SIO_D_PORT_GROUP, SIO_D_PIN)

void SCCB_INIT(void)
{
	gpio_cfg((uint32_t)SIO_C_PORT_GROUP, SIO_C_PIN, GPIO_Mode_Out_PP);
	gpio_cfg((uint32_t)SIO_D_PORT_GROUP, SIO_D_PIN, GPIO_Mode_Out_OD);
}

/*
-----------------------------------------------
   ����: start����,SCCB����ʼ�ź�
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void startSCCB(void)
{
    SCCB_SID_H();     //�����߸ߵ�ƽ
    delay_us(15);

    SCCB_SIC_H();	   //��ʱ���߸ߵ�ʱ���������ɸ�����
    delay_us(15);
 
    SCCB_SID_L();
    delay_us(15);

    SCCB_SIC_L();	 //�����߻ָ��͵�ƽ��������������Ҫ
    delay_us(15);


}
/*
-----------------------------------------------
   ����: stop����,SCCB��ֹͣ�ź�
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void stopSCCB(void)
{
    SCCB_SID_L();
    delay_us(15);
 
    SCCB_SIC_H();	
    delay_us(15);
  

    SCCB_SID_H();	
    delay_us(15);
   
}

/*
-----------------------------------------------
   ����: noAck,����������ȡ�е����һ����������
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void noAck(void)
{
	
	SCCB_SID_H();	
	delay_us(15);
	
	SCCB_SIC_H();	
	delay_us(15);
	
	SCCB_SIC_L();	
	delay_us(15);
	
	SCCB_SID_L();	
	delay_us(15);

}

/*
-----------------------------------------------
   ����: д��һ���ֽڵ����ݵ�SCCB
   ����: д������
 ����ֵ: ���ͳɹ�����1������ʧ�ܷ���0
-----------------------------------------------
*/
unsigned char SCCBwriteByte(unsigned char m_data)
{
	unsigned char j,tem;

	for(j=0;j<8;j++) //ѭ��8�η�������
	{
		if((m_data<<j)&0x80)
		{
			SCCB_SID_H();	
		}
		else
		{
			SCCB_SID_L();	
		}
		delay_us(15);
		SCCB_SIC_H();	
		delay_us(15);
		SCCB_SIC_L();	
		delay_us(15);

	}
	delay_us(15);
	SCCB_SID_IN;/*����SDAΪ����*/
	delay_us(15);
	SCCB_SIC_H();	
	delay_us(15);
	if(SCCB_SID_STATE){tem=0;}   //SDA=1����ʧ�ܣ�����0}
	else {tem=1;}   //SDA=0���ͳɹ�������1
	SCCB_SIC_L();	
	delay_us(15);	
  	SCCB_SID_OUT;/*����SDAΪ���*/

	return (tem);  
}

/*
-----------------------------------------------
   ����: һ���ֽ����ݶ�ȡ���ҷ���
   ����: ��
 ����ֵ: ��ȡ��������
-----------------------------------------------
*/
unsigned char SCCBreadByte(void)
{
	unsigned char read,j;
	read=0x00;
	
	SCCB_SID_IN;/*����SDAΪ����*/
	delay_us(15);
	for(j=8;j>0;j--) //ѭ��8�ν�������
	{		     
		delay_us(15);
		SCCB_SIC_H();
		delay_us(15);
		read=read<<1;
		if(SCCB_SID_STATE) 
		{
			read=read+1;
		}
		SCCB_SIC_L();
		delay_us(15);
	}	
	return(read);
}
