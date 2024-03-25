/*
*********************************************************************************************************
*
*	ģ������ : ģ�⴮��FIFO����ģ��
*	�ļ����� : bsp_emul_uart_fifo.h
*	��    �� : V1.0
*	˵    �� : ����GPIO+TIM+EXTI+DMA ʵ�ֵ�ģ�⴮�ڵ�

*	Copyright (C), 2024-2030, ifree
*
*********************************************************************************************************
*/

#ifndef _BSP_EMUL_UART_FIFO_H_
#define _BSP_EMUL_UART_FIFO_H_

#define	EMUL_UART1_FIFO_EN		1
#define	EMUL_UART2_FIFO_EN		1

/* ����˿ں� */
typedef enum
{
	ECOM1 = 0,		/* EMUL_UART1 */
	ECOM2 = 1,		/* EMUL_UART2 */
}ECOM_PORT_E;

/* ���崮�ڲ����ʺ�FIFO��������С����Ϊ���ͻ������ͽ��ջ�����, ֧��ȫ˫�� */
#if EMUL_UART1_FIFO_EN == 1
	#define EMUL_UART1_BAUD				115200
	#define EMUL_UART1_TX_BUF_SIZE		1*512
	#define EMUL_UART1_RX_BUF_SIZE		1*512
#endif

#if EMUL_UART2_FIFO_EN == 1
	#define EMUL_UART2_BAUD				9600
	#define EMUL_UART2_TX_BUF_SIZE		1*512
	#define EMUL_UART2_RX_BUF_SIZE		1*512
#endif

/* �����豸�ṹ�� */
typedef struct
{
	UART_Emul_HandleTypeDef 	uartEmulHandle;		/* STM32ģ�⴮���豸ָ�� */
	uint8_t *pTxBuf;			/* ���ͻ����� */
	uint8_t *pRxBuf;			/* ���ջ����� */
	uint16_t usTxBufSize;		/* ���ͻ�������С */
	uint16_t usRxBufSize;		/* ���ջ�������С */
	__IO uint16_t usTxWrite;	/* ���ͻ�����дָ�� */
	__IO uint16_t usTxRead;		/* ���ͻ�������ָ�� */
	__IO uint16_t usTxCount;	/* �ȴ����͵����ݸ��� */

	__IO uint16_t usRxWrite;	/* ���ջ�����дָ�� */
	__IO uint16_t usRxRead;		/* ���ջ�������ָ�� */
	__IO uint16_t usRxCount;	/* ��δ��ȡ�������ݸ��� */

	void (*SendBefor)(void); 	/* ��ʼ����֮ǰ�Ļص�����ָ�루��Ҫ����RS485�л�������ģʽ�� */
	void (*SendOver)(void); 	/* ������ϵĻص�����ָ�루��Ҫ����RS485������ģʽ�л�Ϊ����ģʽ�� */
	void (*ReciveNew)(uint8_t _byte);	/* �����յ����ݵĻص�����ָ�� */
	uint8_t Sending;			/* ���ڷ����� */
}EMUL_UART_T;

void bsp_InitEmulUart(ECOM_PORT_E _ucPort);
void ecomSendBuf(ECOM_PORT_E _ucPort, uint8_t *pBuf, uint32_t bufLen);

#endif
