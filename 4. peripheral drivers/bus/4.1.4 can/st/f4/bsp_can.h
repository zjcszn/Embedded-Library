/*
*********************************************************************************************************
*
*	ģ������ : FDCAN����ģ��
*	�ļ����� : bsp_can.c
*
*	Copyright (C), 2018-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#ifndef __BSP_CAN_H
#define __BSP_CAN_H


void bsp_InitCan1(void);
void bsp_InitCan2(void);


void can1_SendPacket(uint8_t *_DataBuf, uint32_t _Len);
void can2_SendPacket(uint8_t *_DataBuf, uint32_t _Len);

extern uint8_t g_Can1RxData[8];
extern uint8_t g_Can1TxData[8];

extern uint8_t g_Can2RxData[8];
extern uint8_t g_Can2TxData[8];


#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
