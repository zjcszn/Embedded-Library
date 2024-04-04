/*********************************************************************
*                                                                    *
*   Copyright (c) 2010 Shanghai FuDan MicroElectronic Inc, Ltd.      *
*   All rights reserved. Licensed Software Material.                 *
*                                                                    *
*   Unauthorized use, duplication, or distribution is strictly       *
*   prohibited by law.                                               *
*                                                                    *
*********************************************************************/
#ifndef _LPCD_CFG_H
#define _LPCD_CFG_H

#define LPCD_TIMER1_CFG   		8   //LPCD���߽׶�ʱ������
//0 = 0�룻1 = 300���룻2 = 400���룻3 = 500���룻4 = 600���룻5 = 700���룻6 = 800���룻7 = 900���룻
//8 = 1�룻9 = 1.1�룻10 = 1.2�룻11 = 1.3�룻12 = 1.4�룻13 = 1.5�룻14 = 1.6�룻15 = 1.7�룻

#define LPCD_TIMER2_CFG  	  	13	//LPCD׼���׶�ʱ������
//0 = 0ms��1 = 0ms��2 = 0.4ms��3 = 0.5ms��4 = 0.6ms��5 = 0.7ms��6 = 0.8ms��7 = 0.9ms��8 = 1.0ms��
//9 = 1.1ms��10 = 1.2ms��11 = 1.3ms��12 = 1.4ms��13 = 1.5ms��14 = 1.6ms��15 = 1.7ms��16 = 1.8ms��
//17 = 1.9ms��18 = 2.0ms��19 = 2.1ms��20 = 2.2ms��21 = 2.3ms��22 = 2.4ms��23 = 2.5ms��24 = 2.6ms��
//25 = 2.7ms��26 = 2.8ms��27 = 2.9ms��28 = 3.0ms��29 = 3.1ms��30 = 3.2ms��31 = 3.3ms��

#define LPCD_TIMER3_CFG  	  	12	//LPCD���׶�ʱ������
//0 = 0us��1 = 0us��2 = 4.7us��3 = 9.4us��4 = 14.1us��5 = 18.8us��6 = 23.5us��7 = 28.2us��8 = 32.9us��
//9 = 37.6us��10 = 42.3us��11 = 47us��12 = 51.7us��13 = 56.8us��14 = 61.1us��15 = 65.8us��16 = 70.5us��
//17 = 75.2us��18 = 79.9us��19 = 84.6us��20 = 89.3us��21 = 94us��22 = 98.7us��23 = 103.4us��24 = 108.1us��
//25 = 112.8us��26 = 117.5us��27 = 122.2us��28 = 126.9us��29 = 131.6us��30 = 136.3us��31 = 141us��

#define	LPCD_TIMER_VMID_CFG		13	//VMID׼��ʱ������
//0 = 0ms��1 = 0ms��2 = 0.4ms��3 = 0.5ms��4 = 0.6ms��5 = 0.7ms��6 = 0.8ms��7 = 0.9ms��8 = 1.0ms��
//9 = 1.1ms��10 = 1.2ms��11 = 1.3ms��12 = 1.4ms��13 = 1.5ms��14 = 1.6ms��15 = 1.7ms��16 = 1.8ms��
//17 = 1.9ms��18 = 2.0ms��19 = 2.1ms��20 = 2.2ms��21 = 2.3ms��22 = 2.4ms��23 = 2.5ms��24 = 2.6ms��
//25 = 2.7ms��26 = 2.8ms��27 = 2.9ms��28 = 3.0ms��29 = 3.1ms��30 = 3.2ms��31 = 3.3ms��

#define LPCD_TX2RF_EN		    0	//LPCD����TX2�������ʹ��
//0 = Disable��1 = Enable

#define	LPCD_AUTO_DETECT_TIMES  0   //LPCD����������       
//0 = 3�Σ�1 = 1��

#define	LPCD_AUTO_WUP_EN		0   //�Զ�У׼ʹ��       
//0 = Disable��1 = Enable

#define LPCD_AUTO_WUP_CFG		1   //�Զ�У׼ʱ������       
//0 = 6�룻1 = 12�룻2 = 15���ӣ�3 = 30���ӣ�4 = 1Сʱ��5 = 1.8Сʱ��6 = 3.6Сʱ��7 = 7.2Сʱ

#define LPCD_AUX_EN	            0   //AUXͨ��ʹ��
//0 = Disbale��1 = Enable

#define LPCD_BIAS_CURRENT       3   //ƫ�õ���
//0 = 50nA��1 = 100nA��2 = 150nA��3 = 200nA��4 = 250nA��5 = 300nA��6 = 350nA��7 = 400nA��

//#define LPCD_REFERENCE          4  //0x00~0x7F

#define	LPCD_THRESHOLD	        8 //LPCD�����ֵ����      

#define	LPCD_CALIBRATE_IRQ_TIMEOUT    255 //LPCDУ׼�жϳ�ʱ���� 1~255us

#define LPCD_GAIN_INDEX							10

#define	LPCD_MODE	            0   //LPCDģʽ
//0 = ģʽ0��1 = ģʽ1��2 = ģʽ2

#endif
