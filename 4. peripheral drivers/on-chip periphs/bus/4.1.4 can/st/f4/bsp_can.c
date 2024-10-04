/*
*********************************************************************************************************
*
*	ģ������ : CAN����ģ��
*	�ļ����� : bsp_can.c
*	��    �� : V1.0
*	˵    �� : CAN����. 
*
*	�޸ļ�¼ :
*		�汾��  ����        ����        ˵��
*		V1.0    2023-10-22  eric2013   ��ʽ����
*
*	Copyright (C), 2023-2030, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"


/*
	����CAN1����Ҫ��V6�����ϵ�J12����ñ�̽�PB8��J13����ñ�̽�PB9��
	����CNA2�������κ����á�
*/

/*
*********************************************************************************************************
*                                             CAN1����
*********************************************************************************************************
*/
/* CAN1 GPIO���� */
#define CAN1_TX_PIN       			GPIO_PIN_13
#define CAN1_TX_GPIO_PORT 			GPIOH
#define CAN1_TX_AF        			GPIO_AF9_CAN1
#define CAN1_TX_GPIO_CLK_ENABLE() 	__HAL_RCC_GPIOH_CLK_ENABLE()

#define CAN1_RX_PIN       			GPIO_PIN_9
#define CAN1_RX_GPIO_PORT 			GPIOI
#define CAN1_RX_AF        			GPIO_AF9_CAN1
#define CAN1_RX_GPIO_CLK_ENABLE() 	__HAL_RCC_GPIOI_CLK_ENABLE()

uint8_t g_Can1RxData[8];
uint8_t g_Can1TxData[8];

CAN_HandleTypeDef     Can1Handle;
CAN_TxHeaderTypeDef   CAN1TxHeader;
CAN_RxHeaderTypeDef   CAN1RxHeader;
uint32_t              CAN1TxMailbox;
CAN_FilterTypeDef  	  CAN1FilterConfig;

/*
*********************************************************************************************************
*                                             CAN2����
*********************************************************************************************************
*/
/* CAN2 GPIO���� */
#define CAN2_TX_PIN       			GPIO_PIN_13
#define CAN2_TX_GPIO_PORT 			GPIOB
#define CAN2_TX_AF        			GPIO_AF9_CAN2
#define CAN2_TX_GPIO_CLK_ENABLE() 	__HAL_RCC_GPIOB_CLK_ENABLE()

#define CAN2_RX_PIN       			GPIO_PIN_5
#define CAN2_RX_GPIO_PORT 			GPIOB
#define CAN2_RX_AF       			GPIO_AF9_CAN2
#define CAN2_RX_GPIO_CLK_ENABLE() 	__HAL_RCC_GPIOB_CLK_ENABLE()

uint8_t g_Can2RxData[8];
uint8_t g_Can2TxData[8];

CAN_HandleTypeDef     Can2Handle;
CAN_TxHeaderTypeDef   CAN2TxHeader;
CAN_RxHeaderTypeDef   CAN2RxHeader;
uint32_t              CAN2TxMailbox;
CAN_FilterTypeDef  	  CAN2FilterConfig;

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitCan1
*	����˵��: ��ʼCAN1
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitCan1(void)
{	 
	/*##-1- ����CAN���� #######################################*/
	Can1Handle.Instance = CAN1;

	Can1Handle.Init.TimeTriggeredMode = DISABLE;    /* ��ֹʱ�䴥��ģʽ��������ʱ���) */
	Can1Handle.Init.AutoBusOff = DISABLE;           /* ��ֹ�Զ����߹رչ��� */
	Can1Handle.Init.AutoWakeUp = DISABLE;           /* ��ֹ�Զ�����ģʽ */
	Can1Handle.Init.AutoRetransmission = ENABLE;    /* ��ֹ�ٲö�ʧ��������Զ��ش����� */
	Can1Handle.Init.ReceiveFifoLocked = DISABLE;    /* ��ֹ����FIFO����ģʽ */ 
	Can1Handle.Init.TransmitFifoPriority = DISABLE; /* ��ֹ����FIFO���ȼ� */
	Can1Handle.Init.Mode = CAN_MODE_NORMAL;         /* ����CANΪ��������ģʽ */  

	/* 
		CAN ������ = RCC_APB1Periph_CAN1 / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		�����У�����CAN������Ϊ500kbps		
		CAN ������ = 420000000 / 2 / (1 + 12 + 8) / = 1Mbps		
	*/
	Can1Handle.Init.SyncJumpWidth = CAN_SJW_1TQ; /* ��ΧCAN_SJW_1TQ -- CAN_SJW_4TQ  */
	Can1Handle.Init.TimeSeg1 = CAN_BS1_12TQ;     /* ��ΧCAN_BS1_1TQ -- CAN_BS1_16TQ */
	Can1Handle.Init.TimeSeg2 = CAN_BS2_8TQ;      /* ��ΧCAN_BS2_1TQ -- CAN_BS2_8TQ  */
	Can1Handle.Init.Prescaler = 2;               /* ��Χ1-1024 */

	if (HAL_CAN_Init(&Can1Handle) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-2- ���ù����� ################################################*/
	CAN1FilterConfig.FilterBank = 0;          				/* ������ţ�����28������˫CANʹ�� */
	CAN1FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;   /* ID����ģʽ */
	CAN1FilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;  /* 32bit���� */
	CAN1FilterConfig.FilterIdHigh = 0x0000;                /* ����ID��16bit��1��ID��32bit��16bit */
	CAN1FilterConfig.FilterIdLow = 0x0000;                 /* ����ID��16bit��2��ID��32bit��16bit */
	CAN1FilterConfig.FilterMaskIdHigh = 0x0000;            /* ����λ��16bit��1��ID��32bit��16bit */ 
	CAN1FilterConfig.FilterMaskIdLow = 0x0000;             /* ����λ��16bit��2��ID��32bit��16bit */ 
	CAN1FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;  /* ʹ��FIFO0 */
	CAN1FilterConfig.FilterActivation = ENABLE;            /* ʹ�ܹ��� */
	CAN1FilterConfig.SlaveStartFilterBank = 0;             /* 28�������������ʼ��ַ */  

	if (HAL_CAN_ConfigFilter(&Can1Handle, &CAN1FilterConfig) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-3- ����CAN���� #################################################*/
	if (HAL_CAN_Start(&Can1Handle) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-4- ����FIFO0 Pending�ж� #######################################*/
	if (HAL_CAN_ActivateNotification(&Can1Handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
	
	/*##-5- �������䷢�Ϳ��ж� ##########################################*/
	if (HAL_CAN_ActivateNotification(&Can1Handle, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitCan2
*	����˵��: ��ʼCAN2
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitCan2(void)
{	
	/*##-1- �������� #######################################*/
	Can2Handle.Instance = CAN2;

	Can2Handle.Init.TimeTriggeredMode = DISABLE;    /* ��ֹʱ�䴥��ģʽ��������ʱ���) */
	Can2Handle.Init.AutoBusOff = DISABLE;           /* ��ֹ�Զ����߹رչ��� */
	Can2Handle.Init.AutoWakeUp = DISABLE;           /* ��ֹ�Զ�����ģʽ */
	Can2Handle.Init.AutoRetransmission = ENABLE;    /* ��ֹ�ٲö�ʧ��������Զ��ش����� */
	Can2Handle.Init.ReceiveFifoLocked = DISABLE;    /* ��ֹ����FIFO����ģʽ */ 
	Can2Handle.Init.TransmitFifoPriority = DISABLE; /* ��ֹ����FIFO���ȼ� */
	Can2Handle.Init.Mode = CAN_MODE_NORMAL;         /* ����CANΪ��������ģʽ */  

	/* 
		CAN ������ = RCC_APB1Periph_CAN1 / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		�����У�����CAN������Ϊ500kbps		
		CAN ������ = 420000000 / 2 / (1 + 12 + 8) / = 1Mbps		
	*/
	Can2Handle.Init.SyncJumpWidth = CAN_SJW_1TQ; /* CAN_SJW_1TQ -- CAN_SJW_4TQ  */
	Can2Handle.Init.TimeSeg1 = CAN_BS1_12TQ;     /* CAN_BS1_1TQ -- CAN_BS1_16TQ */
	Can2Handle.Init.TimeSeg2 = CAN_BS2_8TQ;      /* CAN_BS2_1TQ -- CAN_BS2_8TQ  */
	Can2Handle.Init.Prescaler = 2;               /* ��Χ1-1024 */

	if (HAL_CAN_Init(&Can2Handle) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-2- ���ù����� ###########################################*/
	CAN2FilterConfig.FilterBank = 14;          			   /* ������ţ�����28������˫CANʹ�� */
	CAN2FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;   /* ID����ģʽ */
	CAN2FilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;  /* 32bit���� */
	CAN2FilterConfig.FilterIdHigh = 0x0000;                /* ����ID��16bit��1��ID��32bit��16bit */
	CAN2FilterConfig.FilterIdLow = 0x0000;                 /* ����ID��16bit��2��ID��32bit��16bit */
	CAN2FilterConfig.FilterMaskIdHigh = 0x0000;            /* ����λ��16bit��1��ID��32bit��16bit */ 
	CAN2FilterConfig.FilterMaskIdLow = 0x0000;             /* ����λ��16bit��2��ID��32bit��16bit */ 
	CAN2FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO1;  /* ʹ��FIFO0 */
	CAN2FilterConfig.FilterActivation = ENABLE;            /* ʹ�ܹ��� */
	CAN2FilterConfig.SlaveStartFilterBank = 14;            /* 28�������������ʼ��ַ */  

	if (HAL_CAN_ConfigFilter(&Can2Handle, &CAN2FilterConfig) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-3- ����CAN ###########################################*/
	if (HAL_CAN_Start(&Can2Handle) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/*##-4- ʹ��FIFO1 Pending�ж� #######################################*/
	if (HAL_CAN_ActivateNotification(&Can2Handle, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
	
	/*##-5- ʹ�ܷ��Ϳ��ж� #######################################*/
	if (HAL_CAN_ActivateNotification(&Can2Handle, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_CAN_MspInit
*	����˵��: ����CAN gpio
*	��    ��: hcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
	GPIO_InitTypeDef   GPIO_InitStruct;

	/* ʹ��HAL������˫CANһ��Ҫ����ǰ����������GPIO�������ˣ�������ߺ󣬻ᴥ������Wait initialisation acknowledge */
	
	
	/*##-1- ʹ��CANʱ�� #################################*/
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_CAN2_CLK_ENABLE();
	
	/* ʹ��GPIOʱ�� ****************************************/
	CAN1_TX_GPIO_CLK_ENABLE();
	CAN1_RX_GPIO_CLK_ENABLE();

	/*##-2- ����GPIO ##########################################*/
	GPIO_InitStruct.Pin = CAN1_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CAN1_TX_AF;

	HAL_GPIO_Init(CAN1_TX_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CAN1_RX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CAN1_RX_AF;

	HAL_GPIO_Init(CAN1_RX_GPIO_PORT, &GPIO_InitStruct);

	/*##-3- ����NVIC #################################################*/
	HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	
	HAL_NVIC_SetPriority(CAN1_TX_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);

    //////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////	
	
	/*##-1- ʹ��CANʱ�� #################################*/
	__HAL_RCC_CAN1_CLK_ENABLE();
	__HAL_RCC_CAN2_CLK_ENABLE();

	CAN2_TX_GPIO_CLK_ENABLE();
	CAN2_RX_GPIO_CLK_ENABLE();

	/*##-2- ����GPIO ######################################*/
	GPIO_InitStruct.Pin = CAN2_TX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CAN2_TX_AF;

	HAL_GPIO_Init(CAN2_TX_GPIO_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CAN2_RX_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate =  CAN2_RX_AF;

	HAL_GPIO_Init(CAN2_RX_GPIO_PORT, &GPIO_InitStruct);

	/*##-3- ����NVIC #################################################*/
	HAL_NVIC_SetPriority(CAN2_RX1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(CAN2_RX1_IRQn);
	
	HAL_NVIC_SetPriority(CAN2_TX_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_CAN_MspDeInit
*	����˵��: ����CAN gpio, �ָ�Ϊ��ͨGPIO��ȡ���ж�
*	��    ��: hcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		__HAL_RCC_CAN1_FORCE_RESET();
		__HAL_RCC_CAN1_RELEASE_RESET();

		HAL_GPIO_DeInit(CAN1_TX_GPIO_PORT, CAN1_TX_PIN);
		HAL_GPIO_DeInit(CAN1_RX_GPIO_PORT, CAN1_RX_PIN);

		HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
	}
	
	if(hcan == &Can2Handle)
	{
		__HAL_RCC_CAN2_FORCE_RESET();
		__HAL_RCC_CAN2_RELEASE_RESET();

		HAL_GPIO_DeInit(CAN2_TX_GPIO_PORT, CAN2_TX_PIN);
		HAL_GPIO_DeInit(CAN2_RX_GPIO_PORT, CAN2_RX_PIN);

		HAL_NVIC_DisableIRQ(CAN2_RX1_IRQn);
	}	
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_CAN_RxFifo0MsgPendingCallback
*	����˵��: Rx FIFO0�ص�����
*	��    ��: hcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &CAN1RxHeader, g_Can1RxData) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);
		}

		if (CAN1RxHeader.StdId == 0x222)
		{
			bsp_PutMsg(MSG_CAN1_RxFIFO0Pending, 0);
		}	
	}
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_CAN_RxFifo1MsgPendingCallback
*	����˵��: Rx FIFO1�ص�����
*	��    ��: hcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can2Handle)
	{
		if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &CAN2RxHeader, g_Can2RxData) != HAL_OK)
		{
			Error_Handler(__FILE__, __LINE__);
		}

		if (CAN2RxHeader.StdId == 0x222)
		{
			bsp_PutMsg(MSG_CAN2_RxFIFO1Pending, 0);
		}	
	}
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_CAN_TxMailbox0CompleteCallback
*	����˵��: TxMailbox0�ص�����
*	��    ��: hcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		bsp_PutMsg(MSG_CAN1_TxMailBox0, 0);		
	}
	
	if(hcan == &Can2Handle)
	{
		bsp_PutMsg(MSG_CAN2_TxMailBox0, 0);			
	}
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_CAN_TxMailbox1CompleteCallback
*	����˵��: TxMailbox1�ص�����
*	��    ��: hcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		bsp_PutMsg(MSG_CAN1_TxMailBox1, 0);		
	}
	
	if(hcan == &Can2Handle)
	{
		bsp_PutMsg(MSG_CAN2_TxMailBox1, 0);			
	}
}

/*
*********************************************************************************************************
*	�� �� ��: HAL_CAN_TxMailbox2CompleteCallback
*	����˵��: TxMailbox2�ص�����
*	��    ��: hcan
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
	if(hcan == &Can1Handle)
	{
		bsp_PutMsg(MSG_CAN1_TxMailBox2, 0);		
	}
	
	if(hcan == &Can2Handle)
	{
		bsp_PutMsg(MSG_CAN2_TxMailBox2, 0);			
	}
}

/*
*********************************************************************************************************
*	�� �� ��: can1_SendPacket
*	����˵��: ����һ������
*	��    �Σ�_DataBuf ���ݻ�������_Len ���ݳ��� 0 - 8�ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can1_SendPacket(uint8_t *_DataBuf, uint32_t _Len)
{		
	CAN1TxHeader.StdId = 0x222;
	CAN1TxHeader.ExtId = 0x00;
	CAN1TxHeader.RTR = CAN_RTR_DATA; /* ����֡ */
	CAN1TxHeader.IDE = CAN_ID_STD;   /* ��׼ID */
	CAN1TxHeader.DLC = _Len;
	CAN1TxHeader.TransmitGlobalTime = DISABLE;
	
	/* �������� */
	if (HAL_CAN_AddTxMessage(&Can1Handle, &CAN1TxHeader, _DataBuf, &CAN1TxMailbox) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);	  
	}
}

/*
*********************************************************************************************************
*	�� �� ��: can2_SendPacket
*	����˵��: ����һ������
*	��    �Σ�_DataBuf ���ݻ�������_Len ���ݳ��� 0 - 8�ֽ�
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void can2_SendPacket(uint8_t *_DataBuf, uint32_t _Len)
{		
	CAN2TxHeader.StdId = 0x222;
	CAN2TxHeader.ExtId = 0x00;
	CAN2TxHeader.RTR = CAN_RTR_DATA;
	CAN2TxHeader.IDE = CAN_ID_STD;
	CAN2TxHeader.DLC = _Len;
	CAN2TxHeader.TransmitGlobalTime = DISABLE;
	
	/* �������� */
	if (HAL_CAN_AddTxMessage(&Can2Handle, &CAN2TxHeader, _DataBuf, &CAN2TxMailbox) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);	  
	}
}

/*
*********************************************************************************************************
*	�� �� ��: ---
*	����˵��: CAN�жϷ������
*	��    ��: can
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void CAN1_RX0_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&Can1Handle);
}

void CAN1_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&Can1Handle);
}

void CAN2_RX1_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&Can2Handle);
}

void CAN2_TX_IRQHandler(void)
{
	HAL_CAN_IRQHandler(&Can2Handle);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
