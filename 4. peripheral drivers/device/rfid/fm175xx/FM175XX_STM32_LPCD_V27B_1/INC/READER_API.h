#ifndef _READERAPI_H_
#define _READERAPI_H_


static const unsigned char RF_CMD_REQA = 0x26;
static const unsigned char RF_CMD_WUPA	= 0x52;
static const unsigned char RF_CMD_ANTICOL[3] = {0x93,0x95,0x97} ;
static const unsigned char RF_CMD_SELECT[3] = {0x93,0x95,0x97} ;

//�����������
#define MODWIDTH 0x26 //Ĭ������Ϊ 0x26 ��0x26 = 106K��0x13 = 212K�� 0x09 = 424K ��0x04 = 848K

//���ղ�������
#define RXGAIN	4		//���÷�Χ0~7
#define GSNON	15			//���÷�Χ0~15
#define MODGSNON 8 	//���÷�Χ0~15
#define GSP 31			//���÷�Χ0~63
#define MODGSP 31		//���÷�Χ0~63
#define COLLLEVEL 4	//���÷�Χ0~7
#define MINLEVEL  8	//���÷�Χ0~15
#define RXWAIT 4		//���÷�Χ0~63
#define UARTSEL 2		//Ĭ������Ϊ2  ���÷�Χ0~3 0:�̶��͵�ƽ 1:TIN�����ź� 2:�ڲ������ź� 3:TIN�����ź�

struct type_b_struct
{
unsigned char ATQB[12];
unsigned char	PUPI[4];
unsigned char	APPLICATION_DATA[4];
unsigned char	PROTOCOL_INF[3];
unsigned char ATTRIB[10];
unsigned char UID[8];
unsigned char LEN_ATTRIB;	
};

extern struct type_b_struct Type_B; 

struct type_a_struct
{
unsigned char ATQA[2];
unsigned char CASCADE_LEVEL;
unsigned char UID[15];
unsigned char BCC[3];
unsigned char SAK[3];
};

extern struct type_a_struct Type_A; 

#define FM175XX_SUCCESS				0x00
#define FM175XX_RESET_ERR			0xF1
#define FM175XX_PARAM_ERR 		0xF2	//�����������
#define FM175XX_TIMER_ERR			0xF3	//���ճ�ʱ
#define FM175XX_COMM_ERR			0xF4	//
#define FM175XX_COLL_ERR			0xF5	//��ͻ����
#define FM175XX_FIFO_ERR			0xF6	//FIFO
#define FM175XX_CRC_ERR				0xF7
#define FM175XX_PARITY_ERR		0xF8
#define FM175XX_PROTOCOL_ERR	0xF9

#define FM175XX_AUTH_ERR	0xE1


#define FM175XX_RATS_ERR 	0xD1
#define FM175XX_PPS_ERR 	0xD2
#define FM175XX_PCB_ERR 	0xD3

typedef struct
{
	unsigned char Cmd;                 	// �������
	unsigned char nBitsToSend;					//׼�����͵�λ��	
	unsigned char nBytesToSend;        	//׼�����͵��ֽ���
	unsigned char nBytesToReceive;			//׼�����յ��ֽ���	
	unsigned char nBytesReceived;      	// �ѽ��յ��ֽ���
	unsigned char nBitsReceived;       	// �ѽ��յ�λ��
	unsigned char *pSendBuf;						//�������ݻ�����
	unsigned char *pReceiveBuf;					//�������ݻ�����
	unsigned char CollPos;             	// ��ײλ��
	unsigned char Error;								// ����״̬
	unsigned char Timeout;							//��ʱʱ��
} command_struct;

extern unsigned char SetTimeOut(unsigned int microseconds);
extern unsigned char Command_Execute(command_struct * comm_status);
extern void FM175XX_Initial_ReaderA(void);
extern void FM175XX_Initial_ReaderB(void);
extern unsigned char ReaderA_Halt(void);
extern unsigned char ReaderA_Request(void);
extern unsigned char ReaderA_Wakeup(void);

extern unsigned char ReaderA_AntiColl(unsigned char size);
extern unsigned char ReaderA_Select(unsigned char size);
extern unsigned char ReaderA_CardActivate(void);
extern void SetRf(unsigned char mode);
extern void SetTxCRC(unsigned char mode);
extern void SetRxCRC(unsigned char mode);
extern unsigned char ReaderB_Request(void);
unsigned char ReaderB_Wakeup(void);
extern unsigned char ReaderB_Attrib(void);
extern unsigned char ReaderB_GetUID(void);
extern unsigned char FM175XX_Polling(unsigned char *polling_card);
#endif

