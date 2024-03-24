#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "type.h"

#define SUPPORT_AUDIO			1	
#define ENABLE_MP3				1	//������63k flash
#define SUPPORT_WEBSERVER		1	//������37k flash
#define MACHTALK_ENABLE			1	//MACHTALKƽ̨Զ�̿���

/*
*********************************************************************************************************
*                                            TASK PRIORITIES
* UCOSÿ���߳����ȼ�������ͬ����������ͳһ����
*********************************************************************************************************
*/
#define WORK_QUEUE_MAX_SIZE 4

enum TASK_PRIO{
TASK_UNUSED_PRIO = 10,
TASK_SDIO_THREAD_PRIO,

TASK_TIMER_TASKLET_PRIO,
RTW_XMIT_THREAD_PRIO,
RTW_RECV_THREAD_PRIO,
RTW_CMD_THREAD_PRIO,
TASK_TCPIP_THREAD_PRIO,
TASK_SYS_TASKLET_PRIO,
//����˳��������Ķ�

//user task
TASK_MONITOR_PRIO,
TASK_MAIN_PRIO,
TASK_ADC_RECV_PKG_PRIO,
TASK_TCP_RECV_PRIO,
TASK_TCP_ACCEPT_PRIO,
TASK_WEB_PRIO,
TASK_TCP_SEND_PRIO,
OS_TASK_TMR_PRIO,
};

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*��λWORD
*********************************************************************************************************
*/
#define TASK_TCPIP_THREAD_STACK_SIZE		(512 + 0)
#define TIMER_TASKLET_STACK_SIZE			(256 + 0)
#define SYS_TASKLET_STACK_SIZE				(256 + 0)
#define MOAL_WROK_STACK_SIZE				(256 + 0)
#define TASK_MONITOR_STACK_SIZE			(128 + 0)
#define TASK_SDIO_STACK_SIZE				(200 + 0)

#define RTW_XMIT_THREAD_STACK_SIZE 			(200 + 0)
//#define RTW_RECV_THREAD_STACK_SIZE 		(512 + 0)
#define RTW_CMD_THREAD_STACK_SIZE 			(480 + 0)
//������ֵ������Ķ�

#define TASK_MAIN_STACK_SIZE				(512 + 0)
#define TASK_ADC_RECV_PKG_STACK_SIZE		(256 + 0)
#if ENABLE_MP3
#define TASK_TCP_RECV_STACK_SIZE			(1024 + 512)
#else
#define TASK_TCP_RECV_STACK_SIZE			(512 + 0)
#endif
#define TASK_TCP_SEND_STACK_SIZE			(256 + 0)
#define TASK_ACCEPT_STACK_SIZE				(128 + 0)
#define TASK_WEB_STK_SIZE					(128 + 0)


/*
*********************************************************************************************************
*                               		IRQ_PRIORITY
*����жϺ�������Ҫ��ӡ��Ϣ������ж����ȼ�Ҫ����uart�ж�
*********************************************************************************************************
*/

enum IRQ_PRIORITY{
	
	USART1_IRQn_Priority = 1,
	SDIO_IRQn_Priority,
	TIM2_IRQn_Priority,
	USART3_IRQn_Priority,
	DMA2_Stream5_IRQn_Priority,
	DMA2_Stream7_IRQn_Priority,
	ADC_IRQn_Priority,
	DMA1_Stream5_IRQn_Priority,
	SPI3_IRQn_Priority,
	DMA2_Stream0_IRQn_Priority,
	TIM3_IRQn_Priority,
	TIM4_IRQn_Priority,
	DMA2_Stream1_IRQn_Priority,
	DCMI_IRQn_Priority,
	SysTick_IRQn_Priority
};

/*
*********************************************************************************************************
*                                      kernel ���泣����Դֵ����
*�������monitor���ȷ�����ʵ���ֵ
*********************************************************************************************************
*/
#define EVENTS_MAX		64		//EVENTS_MAX�������¼�������֮�ͣ��������䡢��Ϣ���С��ź�������������
#define TIMER_MAX       48		//��ʱ��
#define MSG_QUEUE_MAX	16		//��Ϣ����
#define TASKS_MAX		16		//����


/*
*********************************************************************************************************
*                                      �����ж��������ַ
* ���16k��ַ����bootloader�����ǵĴ�����Ҫ��16k��ʼ��������Ҫ������ʱ���ж���������ӳ�䵽16k��ַ
* Ŀǰֻ��F205��Ч��F103�汾��ʹ��bootloader
*********************************************************************************************************
*/
#define IVECTOR_ADDR 					(16*1024)
//#define IVECTOR_ADDR 					(0)		//�����ʹ��bootloaer��ͬʱ��IROM1��start��Ϊ0x8000000��

#endif
