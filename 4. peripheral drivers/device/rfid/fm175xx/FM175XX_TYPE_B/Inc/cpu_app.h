#ifndef CPU_APP_H
#define CPU_APP_H
extern unsigned char TPDU_Send[255],TPDU_Rece[255],TPDU_len;

extern struct 
{
unsigned char send_len;
unsigned char *send_buff;//Ӧ�õ�Ԫ����ָ��
unsigned char rece_len;
unsigned char *rece_buff;//Ӧ�õ�Ԫ����ָ��
}APDU;

extern unsigned char CPU_APP(void);
extern unsigned char APDU_Exchange(unsigned char send_len,unsigned char *send,unsigned char rece_len,unsigned char *rece);

#endif