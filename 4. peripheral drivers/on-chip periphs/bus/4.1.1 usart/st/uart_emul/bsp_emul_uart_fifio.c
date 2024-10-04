/*
*********************************************************************************************************
*
*	ģ������ : ģ�⴮��FIFO����ģ��
*	�ļ����� : bsp_emul_uart_fifo.c
*	��    �� : V1.0
*	˵    �� : ����GPIO+TIM+EXTI+DMA ʵ�ֵ�ģ�⴮�ڵ�

*********************************************************************************************************
*/

#include "bsp.h"
#include "stm32h7xx_hal_uart_emul.h"
#include "bsp_emul_uart_fifio.h"

/*  Enable the clock for port EMUL_UART1 */
#define EMUL_UART1_CLK_ENABLE()				   __HAL_RCC_TIM1_CLK_ENABLE();\
											   __HAL_RCC_DMA2_CLK_ENABLE();
											   
#define EMUL_UART1_TX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOD_CLK_ENABLE()
#define EMUL_UART1_RX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOD_CLK_ENABLE()

/* Initialize GPIO and pin number for EMUL_UART1 */
#define EMUL_UART1_TX_PIN			GPIO_PIN_14
#define EMUL_UART1_TX_PORT			GPIOD
#define EMUL_UART1_RX_PIN			GPIO_PIN_15
#define EMUL_UART1_RX_PORT			GPIOD

/* Definition for EMUL_UART1  NVIC */
#define EMUL_UART1_IRQHandler	EXTI15_10_IRQHandler
#define EMUL_UART1_EXTI_IRQ		EXTI15_10_IRQn

/*  Enable the clock for port EMUL_UART2 */
#define EMUL_UART2_CLK_ENABLE()				   __HAL_RCC_TIM1_CLK_ENABLE();\
											   __HAL_RCC_DMA2_CLK_ENABLE();
											   
#define EMUL_UART2_TX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()
#define EMUL_UART2_RX_GPIO_CLK_ENABLE()        __HAL_RCC_GPIOB_CLK_ENABLE()

/* Initialize GPIO and pin number for EMUL_UART2 */
#define EMUL_UART2_TX_PIN			GPIO_PIN_15
#define EMUL_UART2_TX_PORT			GPIOB
#define EMUL_UART2_RX_PIN			GPIO_PIN_14
#define EMUL_UART2_RX_PORT			GPIOB

/* Definition for EMUL_UART2  NVIC */
#define EMUL_UART2_EXTI_IRQHandler	EXTI15_10_IRQHandler
#define EMUL_UART2_EXTI_IRQ			EXTI15_10_IRQn

/* ����ÿ�����ڽṹ����� */
#if EMUL_UART1_FIFO_EN == 1
	static EMUL_UART_T g_tEmulUart1;
	static uint8_t g_EmulTxBuf1[UART1_TX_BUF_SIZE];		/* ���ͻ����� */
	static uint8_t g_EmulRxBuf1[UART1_RX_BUF_SIZE];		/* ���ջ����� */
#endif

#if EMUL_UART2_FIFO_EN == 1
	static EMUL_UART_T g_tEmulUart2;
	static uint8_t g_EmulTxBuf2[UART1_TX_BUF_SIZE];		/* ���ͻ����� */
	static uint8_t g_EmulRxBuf2[UART1_RX_BUF_SIZE];		/* ���ջ����� */
#endif

static void EmulUartVarInit(ECOM_PORT_E _ucPort);
static void InitHardEmulUart(ECOM_PORT_E _ucPort);

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitEmulUart
*	����˵��: ��ʼ��ģ�⴮��Ӳ��������ȫ�ֱ�������ֵ.
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitEmulUart(ECOM_PORT_E _ucPort)
{	
	EmulUartVarInit(_ucPort);		/* �����ȳ�ʼ��ȫ�ֱ���,������Ӳ�� */
	InitHardEmulUart(_ucPort);		/* ���ô��ڵ�Ӳ������(�����ʵ�) */
}

/*
*********************************************************************************************************
*	�� �� ��: EComToEmulUart
*	����˵��: ��ECOM�˿ں�ת��ΪEMUL_UARTָ��
*	��    ��: _ucPort: �˿ں�(ECOM1 - ECOM2)
*	�� �� ֵ: emul_uartָ��
*********************************************************************************************************
*/
EMUL_UART_T *EComToEmulUart(ECOM_PORT_E _ucPort)
{
	if (_ucPort == ECOM1)
	{
		#if EMUL_UART1_FIFO_EN == 1
			return &g_tEmulUart1;
		#else
			return 0;
		#endif
	}
	else if (_ucPort == ECOM2)
	{
		#if EMUL_UART2_FIFO_EN == 1
			return &g_tEmulUart2;
		#else
			return 0;
		#endif
	}
	else
	{
		Error_Handler(__FILE__, __LINE__);
		return 0;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: bsp_SetUartParam
*	����˵��: ���ô��ڵ�Ӳ�������������ʣ�����λ��ֹͣλ����ʼλ��У��λ���ж�ʹ�ܣ��ʺ���STM32- H7������
*	��    ��: Instance   USART_TypeDef���ͽṹ��
*             BaudRate   ������
*             Parity     У�����ͣ���У�����żУ��
*             Mode       ���ͺͽ���ģʽʹ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_SetEmulUartParam(UART_Emul_HandleTypeDef *pUartEmulHandle, uint32_t BaudRate, uint32_t Parity, uint32_t Mode)
{
	/* ����ģ�⴮��Ӳ������ */
	/*
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - BaudRate = 9600 baud
      - Parity = None
	*/
	pUartEmulHandle->Init.Mode        = Mode;
	pUartEmulHandle->Init.BaudRate    = BaudRate;	
	pUartEmulHandle->Init.Parity      = Parity;
	pUartEmulHandle->Init.StopBits    = UART_EMUL_STOPBITS_1;
	pUartEmulHandle->Init.WordLength  = UART_EMUL_WORDLENGTH_8B;

	if (HAL_UART_Emul_Init(pUartEmulHandle) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}		
}

/*
*********************************************************************************************************
*	�� �� ��: InitHardUart
*	����˵��: ���ô��ڵ�Ӳ�������������ʣ�����λ��ֹͣλ����ʼλ��У��λ���ж�ʹ�ܣ��ʺ���STM32-H7������
*	��    ��: _ucPort: �˿ں�(ECOM1 - ECOM2)
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void InitHardEmulUart(ECOM_PORT_E _ucPort)
{
	GPIO_InitTypeDef   GPIO_InitStruct;	
	if(_ucPort == ECOM1)
	{
		/* Enable clock for UART Emul */
		EMUL_UART1_CLK_ENABLE();

		/* Enable GPIO TX/RX clock */
		EMUL_UART1_TX_GPIO_CLK_ENABLE();
		EMUL_UART1_RX_GPIO_CLK_ENABLE();

		/* Initialize UART Emulation port name */
		g_tEmulUart1.uartEmulHandle.TxPortName = EMUL_UART1_TX_PORT;
		g_tEmulUart1.uartEmulHandle.RxPortName = EMUL_UART1_RX_PORT;

		/*Initialize UART Emulation pin number for Tx */
		g_tEmulUart1.uartEmulHandle.Init.RxPinNumber = EMUL_UART1_RX_PIN;
		g_tEmulUart1.uartEmulHandle.Init.TxPinNumber = EMUL_UART1_TX_PIN;

		/* Configure GPIOE for UART Emulation Tx */
		GPIO_InitStruct.Pin    = EMUL_UART1_TX_PIN;
		GPIO_InitStruct.Mode   = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull   = GPIO_NOPULL;
		GPIO_InitStruct.Speed  = GPIO_SPEED_HIGH;

		HAL_GPIO_Init(EMUL_UART1_TX_PORT, &GPIO_InitStruct);

		/* Configure GPIOC for UART Emulation Rx */
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		GPIO_InitStruct.Pin  = EMUL_UART1_RX_PIN;

		HAL_GPIO_Init(EMUL_UART1_RX_PORT, &GPIO_InitStruct);

		bsp_SetEmulUartParam(&(g_tEmulUart1.uartEmulHandle), EMUL_UART1_BAUD,  UART_EMUL_PARITY_NONE, UART_EMUL_MODE_TX );
		/*##-2- Enable NVIC for line Rx  #################################*/
		/* Enable and set EXTI Line Interrupt to the highest priority */
		HAL_NVIC_SetPriority(EMUL_UART1_EXTI_IRQ, 0, 0);
		HAL_NVIC_EnableIRQ(EMUL_UART1_EXTI_IRQ);		
	}
	
	if(_ucPort == ECOM2)
	{
		/* Enable clock for UART Emul */
		EMUL_UART2_CLK_ENABLE();

		/* Enable GPIO TX/RX clock */
		EMUL_UART2_TX_GPIO_CLK_ENABLE();
		EMUL_UART2_RX_GPIO_CLK_ENABLE();

		/* Initialize UART Emulation port name */
		g_tEmulUart2.uartEmulHandle.TxPortName = EMUL_UART2_TX_PORT;
		g_tEmulUart2.uartEmulHandle.RxPortName = EMUL_UART2_RX_PORT;

		/*Initialize UART Emulation pin number for Tx */
		g_tEmulUart2.uartEmulHandle.Init.RxPinNumber = EMUL_UART2_RX_PIN;
		g_tEmulUart2.uartEmulHandle.Init.TxPinNumber = EMUL_UART2_TX_PIN;

		/* Configure GPIOE for UART Emulation Tx */
		GPIO_InitStruct.Pin    = EMUL_UART2_TX_PIN;
		GPIO_InitStruct.Mode   = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull   = GPIO_NOPULL;
		GPIO_InitStruct.Speed  = GPIO_SPEED_HIGH;

		HAL_GPIO_Init(EMUL_UART2_TX_PORT, &GPIO_InitStruct);

		/* Configure GPIOC for UART Emulation Rx */
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		GPIO_InitStruct.Pin  = EMUL_UART2_RX_PIN;

		HAL_GPIO_Init(EMUL_UART2_RX_PORT, &GPIO_InitStruct);

		bsp_SetEmulUartParam(&(g_tEmulUart2.uartEmulHandle), EMUL_UART2_BAUD,  UART_EMUL_PARITY_NONE, UART_EMUL_MODE_TX );
		/*##-2- Enable NVIC for line Rx  #################################*/
		/* Enable and set EXTI Line Interrupt to the highest priority */
		HAL_NVIC_SetPriority(EMUL_UART2_EXTI_IRQ, 0, 0);
		HAL_NVIC_EnableIRQ(EMUL_UART2_EXTI_IRQ);		
	}
}

/*
*********************************************************************************************************
*	�� �� ��: EmulUartVarInit
*	����˵��: ��ʼ��������صı���
*	��    ��: _ucPort: �˿ں�(ECOM1 - ECOM2)
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void EmulUartVarInit(ECOM_PORT_E _ucPort)
{
	if(_ucPort == ECOM1)
	{
		// g_tEmulUart1.uart = USART1;						/* STM32 �����豸 */
		g_tEmulUart1.pTxBuf = g_EmulTxBuf1;				/* ���ͻ�����ָ�� */
		g_tEmulUart1.pRxBuf = g_EmulRxBuf1;				/* ���ջ�����ָ�� */
		g_tEmulUart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* ���ͻ�������С */
		g_tEmulUart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* ���ջ�������С */
		g_tEmulUart1.usTxWrite = 0;						/* ����FIFOд���� */
		g_tEmulUart1.usTxRead = 0;						/* ����FIFO������ */
		g_tEmulUart1.usRxWrite = 0;						/* ����FIFOд���� */
		g_tEmulUart1.usRxRead = 0;						/* ����FIFO������ */
		g_tEmulUart1.usRxCount = 0;						/* ���յ��������ݸ��� */
		g_tEmulUart1.usTxCount = 0;						/* �����͵����ݸ��� */
		g_tEmulUart1.SendBefor = 0;						/* ��������ǰ�Ļص����� */
		g_tEmulUart1.SendOver = 0;						/* ������Ϻ�Ļص����� */
		g_tEmulUart1.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
		g_tEmulUart1.Sending = 0;						/* ���ڷ����б�־ */		
	}
	
	if(_ucPort == ECOM2)
	{
		// g_tEmulUart1.uart = USART1;						/* STM32 �����豸 */
		g_tEmulUart2.pTxBuf = g_EmulTxBuf2;				/* ���ͻ�����ָ�� */
		g_tEmulUart2.pRxBuf = g_EmulRxBuf2;				/* ���ջ�����ָ�� */
		g_tEmulUart2.usTxBufSize = UART1_TX_BUF_SIZE;	/* ���ͻ�������С */
		g_tEmulUart2.usRxBufSize = UART1_RX_BUF_SIZE;	/* ���ջ�������С */
		g_tEmulUart2.usTxWrite = 0;						/* ����FIFOд���� */
		g_tEmulUart2.usTxRead = 0;						/* ����FIFO������ */
		g_tEmulUart2.usRxWrite = 0;						/* ����FIFOд���� */
		g_tEmulUart2.usRxRead = 0;						/* ����FIFO������ */
		g_tEmulUart2.usRxCount = 0;						/* ���յ��������ݸ��� */
		g_tEmulUart2.usTxCount = 0;						/* �����͵����ݸ��� */
		g_tEmulUart2.SendBefor = 0;						/* ��������ǰ�Ļص����� */
		g_tEmulUart2.SendOver = 0;						/* ������Ϻ�Ļص����� */
		g_tEmulUart2.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
		g_tEmulUart2.Sending = 0;						/* ���ڷ����б�־ */			
	}	
}

void ecomSendBuf(ECOM_PORT_E _ucPort, uint8_t *pBuf, uint32_t bufLen)
{
	if(_ucPort == ECOM1)
	{
		HAL_UART_Emul_Transmit_DMA(&(g_tEmulUart1.uartEmulHandle), pBuf, bufLen);	
	}
	
	if(_ucPort == ECOM2)
	{
		HAL_UART_Emul_Transmit_DMA(&(g_tEmulUart2.uartEmulHandle), pBuf, bufLen);	
	}

}
