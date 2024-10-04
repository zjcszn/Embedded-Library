/*********************************************************************
*                                                                    *
*   Copyright (c) 2010 Shanghai FuDan MicroElectronic Inc, Ltd.      *
*   All rights reserved. Licensed Software Material.                 *
*                                                                    *
*   Unauthorized use, duplication, or distribution is strictly       *
*   prohibited by law.                                               *
*                                                                    *
*********************************************************************/

/********************************************************************/
/* 	FM175XX LPCD API V271������																			*/
/* 	��Ҫ����:						        																		*/
/* 	    LPCDУ׼��Ϊ��׼��ѹУ׼���������У׼��������         			*/
/*      LPCD�жϴ����󣬵���������ֵ����������У׼        					*/
/*      ���LPCD��������ȺͲ�ͬУ׼״̬�µ�һ����                  */
/*      ��ֵMAX����ƫ�����������󴥷�                               */
/* 	����:��ҫ�� 																										*/
/* 	����ʱ��:2017��9��7��																						*/
/* 																																	*/
/********************************************************************/
#include "FM175XX_REG.h"
#include "LPCD_API.h"
#include "LPCD_CFG.h"
#include "FM175XX.h"
#include "UART.h"
#include "DEVICE_CFG.h"
#include "Reader_API.h"
//********************************************************************
//�ڲ������б�
//********************************************************************
unsigned char Lpcd_Set_Timer(void);
unsigned char Lpcd_Set_Gain(unsigned char lpcd_gain);
unsigned char Lpcd_Set_Threshold(unsigned char lpcd_threshold_min,unsigned char lpcd_threshold_max);
unsigned char Lpcd_Set_Driver(unsigned char lpcd_cwp,unsigned char lpcd_cwn,unsigned char lpcd_tx2_en);
unsigned char Lpcd_Set_Reference(unsigned char lpcd_bias_current,unsigned char lpcd_reference);
unsigned char Lpcd_WaitFor_Irq(unsigned char IrqType);
unsigned char Lpcd_Get_Calibration_Value(unsigned char *value);
unsigned char Lpcd_Get_Value(unsigned char *value);
unsigned char Lpcd_Set_Aux(unsigned char mode);
void Lpcd_Calibration_Backup(void);
void Lpcd_Calibration_Restore(void);
unsigned char Lpcd_Debug_Info = 0;
unsigned char Lpcd_Debug_Test = 0;
void Lpcd_Set_IRQ_pin(void);
unsigned char Lpcd_Card_Event(void);
struct lpcd_struct Lpcd;

void Lpcd_Set_IRQ_pin(void)
{
  SetReg(JREG_COMMIEN,BIT7);//IRQ���ŷ������			 
	SetReg(JREG_DIVIEN,BIT7);//IRQ����CMOS���ģʽ��IRQ���Ų���Ҫ����������裩   
}

//***********************************************
//�������ƣ�Lpcd_Reset_Status()
//�������ܣ�LPCD���ܸ�λ
//��ڲ�����
//���ڲ�����SUCCESS��������� 
//***********************************************
unsigned char Lpcd_Reset_Status(void)//LPCD���ܸ�λ
{
    SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_RSTN);  //��λLPCD�Ĵ���
    SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_RSTN); //��λ�ſ�LPCD�Ĵ���
    return SUCCESS;
}

//***********************************************
//�������ƣ�Lpcd_Init_Register()
//�������ܣ�LPCD�Ĵ�����ʼ��
//��ڲ�����
//���ڲ�����SUCCESS���������  
//***********************************************
unsigned char Lpcd_Init_Register(void)
{
  Lpcd_Set_IRQ_pin();
	
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_EN);	//ʹ��LPCD����		
	
    if(LPCD_AUTO_DETECT_TIMES == 1)
        SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_SENSE_1);//����1�μ��
    else
        SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_SENSE_1);//����3�μ��
	
	SetReg_Ext(JREG_LPCD_CTRL3,(LPCD_MODE << 3) + BIT2);//����LpcdCtrl3�Ĵ���,LPCD����ģʽ	,BIT2Ĭ��Ϊ1
	SetReg_Ext(JREG_LPCD_AUTO_WUP_CFG,(LPCD_AUTO_WUP_EN << 3) + LPCD_AUTO_WUP_CFG );//����Auto_Wup_Cfg�Ĵ��� 
                  
	return SUCCESS;
}

//***********************************************
//�������ƣ�Lpcd_Set_Timer()
//�������ܣ�LPCD����Timer
//��ڲ�����
//���ڲ�����SUCCESS���������  
//***********************************************
unsigned char Lpcd_Set_Timer(void)
{
		Lpcd.Timer1 = LPCD_TIMER1_CFG & 0x0F;//TIMER1 = 0x01~0x0F
		Lpcd.Timer2 = LPCD_TIMER2_CFG & 0x1F;//TIMER2 = 0x01~0x1F
		Lpcd.Timer3 = LPCD_TIMER3_CFG & 0x1F;//TIMER3 = 0x03~0x1F
		Lpcd.TimerVmid = LPCD_TIMER_VMID_CFG & 0x1F;
    if (Lpcd.Timer3 > 0xF) //Timer3Cfg�õ�5bit��ѡ��16��Ƶ
        {
        Lpcd.Timer3_Offset = 0x05;
				Lpcd.Timer3_Div  = 2;			//16��Ƶ
				Lpcd.Fullscale_Value =  ((Lpcd.Timer3 - 1)<<3) - Lpcd.Timer3_Offset;	
        }
	else if(Lpcd.Timer3 > 0x7) //Timer3Cfg�õ�4bit��ѡ��8��Ƶ
        {
        Lpcd.Timer3_Offset = 0x0E;
				Lpcd.Timer3_Div  = 1;			//8��Ƶ
				Lpcd.Fullscale_Value =  ((Lpcd.Timer3 - 1)<<4) - Lpcd.Timer3_Offset;	
        }
	else 
        {
        Lpcd.Timer3_Offset = 0x1F;
				Lpcd.Timer3_Div  = 0;			//4��Ƶ
				Lpcd.Fullscale_Value =  ((Lpcd.Timer3 - 1)<<5) - Lpcd.Timer3_Offset;	
        }
	  Lpcd.Calibration.Range_L = LPCD_THRESHOLD;// 
    Lpcd.Calibration.Range_H = Lpcd.Fullscale_Value - LPCD_THRESHOLD;// 
		SetReg_Ext(JREG_LPCD_T1CFG,(Lpcd.Timer3_Div <<4) + Lpcd.Timer1);//����Timer1Cfg�Ĵ���
		SetReg_Ext(JREG_LPCD_T2CFG,Lpcd.Timer2);//����Timer2Cfg�Ĵ���
		SetReg_Ext(JREG_LPCD_T3CFG,Lpcd.Timer3);//����Timer3Cfg�Ĵ���	
    SetReg_Ext(JREG_LPCD_VMIDBD_CFG,Lpcd.TimerVmid);//����VmidBdCfg�Ĵ���
    return SUCCESS;
}

//***********************************************
//�������ƣ�Lpcd_Set_Gain()
//�������ܣ�LPCD���÷��ȼ첨�����źŵķŴ�˥������
//��ڲ�����
//���ڲ�����SUCCESS���������  
//***********************************************
unsigned char Lpcd_Set_Gain(unsigned char lpcd_gain)
{
	lpcd_gain &= 0x1F;
	SetReg_Ext(JREG_LPCD_CTRL4,lpcd_gain);
    return SUCCESS;
}

//***********************************************
//�������ƣ�Lpcd_Set_Threshold()
//�������ܣ�LPCD���ô�����ֵ
//��ڲ�����
//���ڲ�����SUCCESS���������  
//***********************************************
unsigned char Lpcd_Set_Threshold(unsigned char lpcd_threshold_min,unsigned char lpcd_threshold_max)
{
unsigned char temp;
       
	if(lpcd_threshold_max < lpcd_threshold_min)
        {
        temp = lpcd_threshold_min;
        lpcd_threshold_min = lpcd_threshold_max;
        lpcd_threshold_max = temp;
        }
  
  SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_L,(lpcd_threshold_min & 0x3F));//д��THRESHOLD_Min��ֵ��6λ

  SetReg_Ext(JREG_LPCD_THRESHOLD_MIN_H,(lpcd_threshold_min>>6));//д��THRESHOLD_Min��ֵ��2λ

	SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_L,(lpcd_threshold_max & 0x3F));//д��THRESHOLD_Max��ֵ��6λ

	SetReg_Ext(JREG_LPCD_THRESHOLD_MAX_H,(lpcd_threshold_max>>6));//д��THRESHOLD_Max��ֵ��2λ
        
  return SUCCESS;

}

//***********************************************
//�������ƣ�Lpcd_Set_Drive()
//�������ܣ�LPCD�����������
//��ڲ�����
//���ڲ�����SUCCESS���������  
//***********************************************
unsigned char Lpcd_Set_Driver(unsigned char lpcd_cwp,unsigned char lpcd_cwn,unsigned char lpcd_tx2_en)
{
    unsigned char reg_data;
	GetReg(JREG_VERSION,&reg_data);
    if(reg_data == 0x88)//V03�汾оƬ
        {
        if(lpcd_cwn > 1)
            lpcd_cwn = 1;
        lpcd_cwn &= 0x01;
        lpcd_cwp &= 0x07;
        SetReg_Ext(JREG_LPCD_CTRL2,((lpcd_tx2_en << 4) + (lpcd_cwn << 3) + lpcd_cwp));//V03�汾оƬ
        }
    if(reg_data == 0x89)//V03���ϰ汾оƬ
        {
          lpcd_cwn &= 0x0F;
          lpcd_cwp &= 0x07;
          SetReg_Ext(JREG_LPCD_CTRL2,((lpcd_tx2_en<<4) + lpcd_cwp));//V03���ϰ汾оƬ
          SetReg(JREG_GSN, lpcd_cwn << 4); //V03���ϰ汾оƬ
        }
  return SUCCESS;
}

//***********************************************
//�������ƣ�Lpcd_Set_Reference()
//�������ܣ�LPCD���û�׼�źŵĳ������������
//��ڲ�����
//���ڲ�����SUCCESS��У׼�ɹ���ERROR��У׼ʧ��
//***********************************************
unsigned char Lpcd_Set_Reference(unsigned char lpcd_bias_current,unsigned char lpcd_reference)
{
		lpcd_bias_current &= 0x07; //0x00 ~ 0x07 ��׼����
    lpcd_reference &= 0x7F;  //0x00 ~ 0x7F ��׼����
    SetReg_Ext(JREG_LPCD_BIAS_CURRENT,((lpcd_reference  & 0x40)>>1) + (lpcd_bias_current & 0x07));       
    SetReg_Ext(JREG_LPCD_ADC_REFERECE,(lpcd_reference  & 0x3F));
    return SUCCESS;
}

//***********************************************
//�������ƣ�Lpcd_Calibrate_Reference()
//�������ܣ�LPCD����У׼��׼��ѹ
//��ڲ�����
//���ڲ�����SUCCESS��У׼�ɹ���ERROR��У׼ʧ��
//***********************************************
unsigned char Lpcd_Calibrate_Reference(void)
{
    unsigned char i,result; 
    result = ERROR; //��ʼֵ����ΪУ׼ʧ��	
    Lpcd_Reset_Status();//У׼��׼��ѹǰ���ȸ�λ״̬��
    for(i = 0 ;i < 0x7F;i ++)
    {
    Lpcd.Calibration.Reference = 0 + i;//
    Lpcd_Set_Reference(LPCD_BIAS_CURRENT,Lpcd.Calibration.Reference);//
    
    Lpcd.Calibration.Gain_Index = 10;//
    Lpcd_Set_Gain(LPCD_GAIN[Lpcd.Calibration.Gain_Index]);//
    
    Lpcd_Set_Driver(7,15,1);//
    Lpcd_Get_Calibration_Value(&Lpcd.Calibration.Value);
		if(Lpcd_Debug_Info)
			{
			Uart_Send_Msg("-> Refference = "); Uart_Send_Hex(&i,1); //Uart_Send_Msg("\r\n");
			Uart_Send_Msg("; Lpcd.Calibration.Value = "); Uart_Send_Hex(&Lpcd.Calibration.Value,1); Uart_Send_Msg("\r\n");
			}
    if((Lpcd.Calibration.Value == 0)&&(Lpcd.Calibration.Reference != 0))
      {
				Uart_Send_Msg("Calibra Reference Success!\r\n");	
        result = SUCCESS;
        break;//
      }
    if((Lpcd.Calibration.Value == 0)&&(Lpcd.Calibration.Reference == 0))
      {
        Uart_Send_Msg("Calibra Reference Error!\r\n");	
        result = ERROR; //
        break;
      }
    }
   return result;
}
//***********************************************
//�������ƣ�Lpcd_Calibrate_Driver()
//�������ܣ�LPCD����У׼�������
//��ڲ�����
//���ڲ�����SUCCESS���������  
//***********************************************
unsigned char Lpcd_Calibrate_Driver(void)
{
    unsigned char i,j;   
		
    Lpcd.Calibration.Gain_Index = LPCD_GAIN_INDEX ;//      
    
   for(j = 0;j < 11;j++)//
   { 
		Lpcd_Set_Gain(LPCD_GAIN[Lpcd.Calibration.Gain_Index]);// ����Gain
    for(i = 0;i < 7;i ++ )//
       {
       Lpcd.Calibration.Driver_Index = i;
       Lpcd_Set_Driver(LPCD_P_DRIVER[Lpcd.Calibration.Driver_Index],LPCD_N_DRIVER[Lpcd.Calibration.Driver_Index],LPCD_TX2RF_EN);//
       Lpcd_Get_Calibration_Value(&Lpcd.Calibration.Value);//����Driver
			 if(Lpcd_Debug_Info)
			 {				 
       Uart_Send_Msg("-> Driver = "); Uart_Send_Hex(&i,1); //Uart_Send_Msg("\r\n");
       Uart_Send_Msg("; Lpcd.Calibration.Value = "); Uart_Send_Hex(&Lpcd.Calibration.Value,1); Uart_Send_Msg("\r\n");
			 } 
       if((Lpcd.Calibration.Value > Lpcd.Calibration.Range_L)&&(Lpcd.Calibration.Value < Lpcd.Calibration.Range_H))
         {
           if((Lpcd.Calibration.Value - LPCD_THRESHOLD > 0 )&&((Lpcd.Calibration.Value + LPCD_THRESHOLD) < Lpcd.Fullscale_Value))//
             
           {
               Lpcd.Calibration.Threshold_Max = Lpcd.Calibration.Value + LPCD_THRESHOLD;
               Lpcd.Calibration.Threshold_Min = Lpcd.Calibration.Value - LPCD_THRESHOLD;
						 
               Uart_Send_Msg("-> Lpcd.Calibration.Value = "); Uart_Send_Hex(&Lpcd.Calibration.Value,1); //Uart_Send_Msg("\r\n");//
						   Uart_Send_Msg("; Lpcd.Fullscale_Value = "); Uart_Send_Hex(&Lpcd.Fullscale_Value,1); Uart_Send_Msg("\r\n");
						   Uart_Send_Msg("-> Driver = "); Uart_Send_Hex(&Lpcd.Calibration.Driver_Index,1);Uart_Send_Msg("; Gain = ");  Uart_Send_Hex(&Lpcd.Calibration.Gain_Index,1); Uart_Send_Msg("\r\n");
               Uart_Send_Msg("-> Lpcd.Calibration.Threshold_Max = "); Uart_Send_Hex(&Lpcd.Calibration.Threshold_Max,1); //
               Uart_Send_Msg("; Lpcd.Calibration.Threshold_Min = "); Uart_Send_Hex(&Lpcd.Calibration.Threshold_Min,1); Uart_Send_Msg("\r\n");//
               Lpcd_Set_Threshold(Lpcd.Calibration.Threshold_Min,Lpcd.Calibration.Threshold_Max);//
						   if(Lpcd_Debug_Test)
								 {
									while(1)//��ѭ��Ϊ���Դ��룬������ʱʹ��
										 {
											 Lpcd_Get_Calibration_Value(&Lpcd.Calibration.Value);//
											 Uart_Send_Msg("-> Lpcd.Calibration.Value2 = "); Uart_Send_Hex(&Lpcd.Calibration.Value,1); Uart_Send_Msg("\r\n");
											 mDelay(500);
										 }
								 }
               Uart_Send_Msg("Calibra Driver Success!\r\n");	        
               return SUCCESS;
             }
         }	 
				 
				 
       }
			 
			  if(Lpcd.Calibration.Value > Lpcd.Calibration.Range_H)//
        {
          if(Lpcd.Calibration.Gain_Index == 11)
            break;//
          else
            Lpcd.Calibration.Gain_Index++;
           Uart_Send_Msg("Gain_Index++\r\n");
        }
      if(Lpcd.Calibration.Value < Lpcd.Calibration.Range_L)//
        {     
          if(Lpcd.Calibration.Gain_Index == 0)
            break;//
          else
             Lpcd.Calibration.Gain_Index--;
           Uart_Send_Msg("Gain_Index--\r\n");
        }
				
		}		
    Uart_Send_Msg("Calibra Driver Error !\r\n");//
    return ERROR;
}

//***********************************************
//�������ƣ�WaitForLpcdIrq()
//�������ܣ��ȴ�LPCD�ж�
//��ڲ�����IrqType
//���ڲ�����SUCCESS����ȡ�������   ERROR����ʱ�˳�
//***********************************************
unsigned char Lpcd_WaitFor_Irq(unsigned char IrqType)
{
	unsigned char ExtRegData;
	unsigned char TimeOutCount;
	
	TimeOutCount = 0;
	GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);

	for(TimeOutCount = LPCD_CALIBRATE_IRQ_TIMEOUT; TimeOutCount > 0; TimeOutCount--)
    {
		uDelay(1);	//��ʱ1us
		GetReg_Ext(JREG_LPCD_IRQ,&ExtRegData);	
		if(ExtRegData & IrqType)
			return SUCCESS; 
    }        
	return ERROR;	
}

//***********************************************
//�������ƣ�Lpcd_Get_Calibration_Value()
//�������ܣ�����һ��У׼����ȡLPCD��ֵ
//��ڲ����� *value
//���ڲ�����SUCCESS����ȡ�������   ERROR:������ʱ
//***********************************************
unsigned char Lpcd_Get_Calibration_Value(unsigned char *value)
{
	unsigned char result;
      	    
  SetReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIBRATE_VMID_EN);//ʹ��VMID��Դ��BFL_JBIT_AMP_EN_SEL = 1 ��ǰʹ��AMP	Lpcd_Reset_Status();  //���CalibraIRq��־
  SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_CALIBRATE_EN);//�ر�У׼ģʽ
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_CALIBRATE_EN);//ʹ��У׼ģʽ
	
	result = Lpcd_WaitFor_Irq(JBIT_CALIBRATE_IRQ);//�ȴ�У׼�����ж�      

  ModifyReg_Ext(JREG_LPCD_MISC,BFL_JBIT_CALIBRATE_VMID_EN,0);//�ر�VMID��Դ 
	
	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_CALIBRATE_EN);//�ر�У׼ģʽ
	
	Lpcd_Get_Value(&*value);
	
  Lpcd_Reset_Status();  //���CalibraIRq��־

	return result;
}

//***********************************************
//�������ƣ�Lpcd_Get_Value()
//�������ܣ���ȡLPCD����ֵ
//��ڲ����� *value
//���ڲ�����SUCCESS����ȡ�������   ERROR:������ʱ
//***********************************************

unsigned char Lpcd_Get_Value(unsigned char *value)
{
	unsigned char ExtRegData;
	GetReg_Ext(JREG_LPCD_ADC_RESULT_H,&ExtRegData);//��ȡ������Ϣ����2λ
	
	*value = ((ExtRegData & 0x3) << 6);

	GetReg_Ext(JREG_LPCD_ADC_RESULT_L,&ExtRegData);//��ȡ������Ϣ����6λ

	*value += (ExtRegData & 0x3F);  

	return SUCCESS;
}

//***********************************************
//�������ƣ�Lpcd_IRQ_Event()
//�������ܣ�LPCD�жϴ���
//��ڲ�����
//���ڲ�����
//***********************************************
void Lpcd_IRQ_Event(void)
{	
	LED_ARM_WORKING_ON;		
	
	if(Lpcd.Irq & JBIT_AUTO_WUP_IRQ)
		{
			Lpcd_Calibration_Event();
		}
	if(Lpcd.Irq & JBIT_CARD_IN_IRQ)//����п�Ƭ�жϣ������������
        {
					if((Lpcd.Value + LPCD_THRESHOLD)> Lpcd.Fullscale_Value )
						Lpcd.Calibration.Threshold_Max = Lpcd.Fullscale_Value;
					else						
						Lpcd.Calibration.Threshold_Max = Lpcd.Value + LPCD_THRESHOLD;
          if((Lpcd.Value - LPCD_THRESHOLD) > 0)
						Lpcd.Calibration.Threshold_Min = Lpcd.Value - LPCD_THRESHOLD;
          else
            Lpcd.Calibration.Threshold_Min = 0;
         
          Uart_Send_Msg("-> Lpcd.Calibration.Threshold_Max = "); Uart_Send_Hex(&Lpcd.Calibration.Threshold_Max,1); //
          Uart_Send_Msg("; Lpcd.Calibration.Threshold_Min = "); Uart_Send_Hex(&Lpcd.Calibration.Threshold_Min,1); Uart_Send_Msg("\r\n");//
          Lpcd_Set_Threshold(Lpcd.Calibration.Threshold_Min,Lpcd.Calibration.Threshold_Max);//LPCD���ô�����ֵ 
					Lpcd_Card_Event();//��������						
	
				}   
	 Lpcd_Set_Driver(LPCD_P_DRIVER[Lpcd.Calibration.Driver_Index],LPCD_N_DRIVER[Lpcd.Calibration.Driver_Index],LPCD_TX2RF_EN);//����LPCD�������
	 Lpcd_Set_Reference(LPCD_BIAS_CURRENT,Lpcd.Calibration.Reference);//����LPCD��׼����
	 Lpcd_Set_Gain(LPCD_GAIN[Lpcd.Calibration.Gain_Index]);//����LPCD��������
	 Lpcd_Set_IRQ_pin();				
	 Lpcd.Irq = 0;//���LPCD�жϱ�־
		
   LED_ARM_WORKING_OFF;   	
	return;	
}

void Lpcd_Calibration_Restore(void)
{
	Lpcd.Calibration.Reference = Lpcd.Calibration_Backup.Reference;
	Lpcd.Calibration.Gain_Index = Lpcd.Calibration_Backup.Gain_Index;
  Lpcd.Calibration.Driver_Index = Lpcd.Calibration_Backup.Driver_Index;
	Uart_Send_Msg("-> Restore Success!\r\n");		
	return;
}

void Lpcd_Calibration_Backup(void)
{
	Lpcd.Calibration_Backup.Reference = Lpcd.Calibration.Reference;
	Lpcd.Calibration_Backup.Gain_Index = Lpcd.Calibration.Gain_Index;
  Lpcd.Calibration_Backup.Driver_Index = Lpcd.Calibration.Driver_Index;
	Uart_Send_Msg("-> Bakckup Success!\r\n");		
	return;
}
//***********************************************
//�������ƣ�Lpcd_Calibration_Event()
//�������ܣ�LCPDУ׼����
//��ڲ�������
//���ڲ�����SUCCESS:У׼�ɹ���ERROR��У׼ʧ��
//***********************************************
unsigned char Lpcd_Calibration_Event(void)
{
	unsigned char result;
  
	Lpcd_Init_Register();		//
	Lpcd_Set_Timer();
	Lpcd_Set_Aux(LPCD_AUX_EN);	//??AUX????

  Uart_Send_Msg("Start Calibration!\r\n");    
  Lpcd_Reset_Status();
  result = Lpcd_Calibrate_Reference();
     if(result == ERROR)
      return ERROR;
   
	result = Lpcd_Calibrate_Driver();
   
		if (result == SUCCESS)		
      {	
			  Uart_Send_Msg("Calibration Success!\r\n");
        return SUCCESS;
			} 
		else
			{
			  Uart_Send_Msg("Calibration Error!\r\n");
				Lpcd_Calibration_Restore();//�ָ�У׼����
			}	 
     
return ERROR;
}

//***********************************************
//�������ƣ�Lpcd_Set_Mode()
//�������ܣ�LCPD����ģʽ����
//��ڲ�����mode = 1:���� mode = 0 :�ر�
//���ڲ�����
//***********************************************
void Lpcd_Set_Mode(unsigned char mode)
{
    if(mode == 1)
        {
        Lpcd_Set_IE(1);//FM175XX��LPCD�ж�               
        FM175XX_HPD(0);//����LPCDģʽ	 
        Uart_Send_Msg("LPCD Mode Entered!\r\n\r\n");       
				EXTI15_10_IRQ_CTRL(ENABLE);//MCU���ж�
        }
    else
        {               
				FM175XX_HPD(1);//�˳�LPCDģʽ	
        Lpcd_Set_IE(0);//FM75XX�ر�LPCD�ж�
        EXTI15_10_IRQ_CTRL(DISABLE);//MCU�ر��ж�
        Uart_Send_Msg("LPCD Mode Exited!\r\n");  
        }     
    return;
}

//***********************************************
//�������ƣ�Lpcd_Get_IRQ()
//�������ܣ�LPCD��ȡ�жϱ�־
//��ڲ�����*irq IRQ��־
//���ڲ���:
//***********************************************
void Lpcd_Get_IRQ(unsigned char *irq)
{
  GetReg_Ext(JREG_LPCD_IRQ,&(*irq));//��ȡLPCD�жϼĴ���

	Uart_Send_Msg("-> Lpcd.Irq = "); Uart_Send_Hex(&*irq,1); 
	if(Lpcd.Irq & JBIT_CARD_IN_IRQ)
	{
		Lpcd_Get_Value(&Lpcd.Value);
		Uart_Send_Msg("; Lpcd.Value = "); Uart_Send_Hex(&Lpcd.Value,1);
	}
	Uart_Send_Msg("\r\n");
	Lpcd_Reset_Status();//���FM175XX�ж�״̬
	return;
}
//***********************************************
//�������ƣ�Lpcd_Set_IE()
//�������ܣ�LPCD�ж���������
//��ڲ�����mode = 1:���� mode = 0 ���ر�
//���ڲ�����SUCCESS���������
//***********************************************
unsigned char Lpcd_Set_IE(unsigned char mode)
{
    if(mode == 1)
        {
       	SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_SET + JBIT_LPCD_IE);//��LPCD�ж�					
        }
    else
        {        
        SetReg_Ext(JREG_LPCD_CTRL1,JBIT_BIT_CTRL_CLR + JBIT_LPCD_IE);//�ر�LPCD�ж�			
        }
    return SUCCESS;
}

//***********************************************
//�������ƣ�Lpcd_Set_Aux()
//�������ܣ�LPCD����ͨ������
//��ڲ�����mode = 1:���� mode = 0 ���ر�
//���ڲ�����SUCCESS���������
//***********************************************

unsigned char Lpcd_Set_Aux(unsigned char mode)
{
	if (mode == 1) 
        {
        ModifyReg_Ext(0x39,BIT7,1);			
				ModifyReg_Ext(0x3C,BIT1,0);	
				ModifyReg_Ext(0x3D,(BIT2 | BIT1 | BIT0),0);				
				ModifyReg_Ext(0x3E,(BIT1 | BIT0),0);
				SetReg(JREG_ANALOGTEST,0x00);
				SetReg_Ext(0x13,BIT0);//Aux1(pin19) = V_demodulation����첨�ź����
				SetReg_Ext(0x14,BIT1);//Aux2(pin20)= Reference��ѹ�ź����
				SetReg_Ext(0x15,0x00);//D2���LPCD_OUT
				SetReg_Ext(0x16,(BIT4 | BIT3 ));//D1(pin25) = T3 �� D2(pin26) = LPCD_OUT    
				SetReg(JREG_TESTPINEN,0x86);//ʹ��D1��D2 ���             
        }
	else
        {   
				SetReg_Ext(0x13,0x00);
				SetReg_Ext(0x14,0x00);
        SetReg_Ext(0x15,0x00);	
        SetReg_Ext(0x16,0x00);	
        SetReg(JREG_TESTPINEN,0x80);//�ر�D1��D2 ���
        }
	return SUCCESS;
}

//***********************************************
//�������ƣ�Lpcd_Card_Event()
//�������ܣ�LPCD��Ƭ����
//��ڲ�����
//���ڲ�����SUCCESS���������
//***********************************************
unsigned char Lpcd_Card_Event(void)
{
	unsigned char card_type,result;
				FM175XX_SoftReset();//??????????	
					SetRf(3);		
					result = FM175XX_Polling(&card_type);				
					SetRf(0);
				
					if(result == SUCCESS)			
						{		
						if(card_type & BIT0)
								{
								Uart_Send_Msg("-> TYPE_A Card Detected!\r\n");	
								Uart_Send_Msg("-> ATQA = ");Uart_Send_Hex(Type_A.ATQA, 2);//Uart_Send_Msg("\r\n");					
								Uart_Send_Msg("; UID = ");Uart_Send_Hex(Type_A.UID, 4 * Type_A.CASCADE_LEVEL);//Uart_Send_Msg("\r\n");					
								Uart_Send_Msg("; SAK = ");Uart_Send_Hex(Type_A.SAK, Type_A.CASCADE_LEVEL);Uart_Send_Msg("\r\n");
								}							
						if(card_type & BIT1)
								{		
									Uart_Send_Msg("-> TYPE_B Card Detected!\r\n");
									Uart_Send_Msg("-> ATQB = ");Uart_Send_Hex(Type_B.ATQB,12);Uart_Send_Msg("\r\n");	
									Uart_Send_Msg("-> UID = ");
									Uart_Send_Hex(Type_B.UID,8);
									Uart_Send_Msg("\r\n");
								}
						}
												
					else
						{
						Uart_Send_Msg("-> No Card Detected!\r\n");
						}
	return result;
}

