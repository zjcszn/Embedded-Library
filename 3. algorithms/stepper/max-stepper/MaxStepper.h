/*******************************************************************************
                        �ൺ����������Ƭ�����޹�˾
                            ��Ȩ����  @ 2014
    
    MaxStepper��������������������ŷ����.����źŷ�Ϊ�����ź�Pls�ͷ�������Dir.
����Ӳ���������������ŷ����������.

������
2014.10.08
*******************************************************************************/
#ifndef  _MAX_STEPPER_H_
#define  _MAX_STEPPER_H_



#include "MaxDuino.h"
#include "MaxTimer.h"


#define  DIR_FOREWARD           0   //��ת
#define  DIR_REVERSAL           1   //��ת

// Ϊ�˱��ڹ����ϵ�ͬ�¶�ȡ���� ʹ��ƴ�����
// 2018.10.01
#define  DIR_ZHENG              DIR_FOREWARD
#define  DIR_FAN                DIR_REVERSAL


#define  RAMP_MODE_S            0   //�Ӽ���ģʽ.S���߼Ӽ���.
#define  RAMP_MODE_L            1   //ֱ�߼Ӽ���.


#define  DEFAULT_MIN_PLS_FREQ   10  //Ĭ���������Ƶ��.


#define  MOTOR_STOP             0   //���ֹͣ��.
#define  MOTOR_UP               1   //���������.
#define  MOTOR_RUN              2   //���������.
#define  MOTOR_DOWN             3   //���������.
#define  MOTOR_ALARM            4   //����


struct STRUCT_RAMP
{
    Uchar    EnRamp;                //б�¹���ʹ�ܱ�־.
    Uchar    RampMode;              //б��ģʽ.
	Ulong    RampLength;            //б�º���--���ٵĳ���.  
    
    Ulong    RampLengthSET;         //�û����õ�300RPM�µļӼ��ٳ���
};

struct STRUCT_SYSTEM
{
    Ulong   RemainSteps;            //ʣ�ಽ��.
    Ulong   CurrentSteps;           //��ǰ����.
    
	float   CurFrq;                 //��¼��ǰ�ٶ�.
	Uchar   State;                  //�������״̬(�Ӽ���״̬).
    
    long    location;		        //��¼��ǰλ��.
    
    Uchar   LSW_PosFlg;		        //��λ���ض�����־λ(������)
    Uchar   LSW_NagFlg;		        //��λ���ض�����־λ(������)
    
    Uchar   SoftLSW_PosFlg;         //�����λ����(������)
    Uchar   SoftLSW_NagFlg;         //�����λ����(������)
    
    Uchar   TimerShareFlg;          //��ʱ������
};

struct STRUCT_CONFIG
{
    MaxTimer    DrvTimer;           //����һ���������ʹ�õĶ�ʱ����������.
    
    Uchar   PIN_LswPos;             //������λ��������
	Uchar   PIN_LswNag;             //������λ��������
    
    Uint    PIN_Step;               //��������.��������.
    Uint    PIN_Dir;                //��������.��������.
    
    Uint    PlsPerCir;              // �������ת��һȦ,����Ҫ����������.
    
    Uchar   DirOpp;		            //�˶������Ƿ���.
    Uchar   RunDir;		            //�����ת������.
    
    float   RPM;                    //�����ת��._RPM.    
    float   TargetFrq;              //Ŀ��Ƶ��.
    
    Uchar   StopLevel;              //���������ֹͣ��תʱ,����ĵ�ƽ.
                                    //Ĭ�ϸߵ�ƽ.

    Uchar   EnLsw;                  //��λ����ʹ�ܱ�־λ.
    Uchar   LswLevel;               //����λ���ض���ʱ�ĵ�ƽ.Ĭ�ϸߵ�ƽ.
	
    Uchar   EnSoftLsw;              //ʹ�������λ
    long    SoftLswMax;             //�����λ.���ֵ.
    long    SoftLswMin;             //�����λ.��Сֵ.
    
    float   MinPlsFrq;              //��������������������Ƶ��.
    
    Uchar   TimerShareEnable;       //ʹ�ܶ�ʱ�����ù���
};


class MaxStepper 
{
  public:

	void  begin( TIM_TypeDef* myTimer, 
                 Uint  myPlusPerCir, 
                 Uint  myPls,  Uint  myDir,
                 Ulong PlsMod, Ulong DirMod,
                 Uchar PlsStopLvl );
    void  begin( TIM_TypeDef* myTimer, 
                 Uint myPlusPerCir, 
                 Uchar myPls, Uchar myDir );


	void  speed(float _RPM);               		//�����ٶ�.��λ:_RPM.
    void  Speed(float _RPM);
    float speed(void);
    float Speed(void);
    
    void  MinSpeed(float _RPM);              	//����ϵͳ�����ת��.�Ӽ��ٵ�ʱ��������.
    float CurSpeed();                           //��ȡ��ǰ�ٶ�.��λ:_RPM.

	void  run(Uchar Dir, Ulong Steps);       	//�������һ���Ĳ���.
    void  run(long Steps);
    void  Run(Uchar Dir, Ulong Steps);
    void  Run(long Steps);
    
	void  stop(void);                        	//����������ֹͣ���.
    void  Stop(void);
    
    void  RStop(void);                       	//б��ֹͣ����.

    Uchar state(void);                      	//��ȡ�������״̬.
    Uchar State(void);
    
    void  wait(void);							//�ȴ����ֹͣ.
    void  wait(Ulong OVTime);                   //����ʱ�ĵȴ�.

    Uchar swait(Uchar ulPin,Uchar Level);       //�ȴ����ֹͣ,ָ���˿ڶ����˳�.
    Uchar swait(Uchar ulPin,Uchar Level,Uint FilterTime);

    void  LSW(Uchar _Cmd);
	void  LSWPin(Uchar myPin1, Uchar myPin2);   //��λ���ز�������.
	void  LSWLevel(Uchar mySetLevel);			//�ӽ����ض���ʱ�ĵ�ƽ.
    void  LSWResetAlarm(void);                  //��λ�ӽ����صı���
    
	void  EnRamp(Uchar myCMD);	                //����б�¹���.    
	void  RampLength(Ulong myRampLength);       //б�º�������.
    void  RampMode(Uchar _Mode);                //�Ӽ���ģʽ.
    
    void  SoftLimit(Uchar _Cmd);                //���������λ
    Uchar SoftLimit(void);                      //��ȡ�����λ��ʹ��״̬
    void  SetSoftLimitMin(long MinValue);       //���������λ����Сֵ
    void  SetSoftLimitMax(long MinValue);       //���������λ�����ֵ
    void  SoftLimitResetAlarm(void);            //��λ�����λ�ı���
    
    void  EnableTimerShare(Uchar _Cmd);         //������ʱ��������
    
	long  location();							//��ȡ��ǰλ��.
	void  location(long CurLocation);			//��ȡ��ǰλ��.
	
	void  DirOpp(Uchar _Cmd);					//�˶������Ƿ���.
    Uchar DirOpp(void);                         //��ȡ�˶������Ƿ���
	Uchar CurDir(void);                         //��ȡ��ǰ�˶�����.
    
	void  MoveTo(long TargetLocation);			//�ƶ���Ŀ��ȫ��λ��.
	
    void  scan(void);		//���������������.��ISR������.
							//�����û�����.
    
    MaxStepper(void);		//���캯��.��ɱ�����ʼ��.
	
	
private:

    MaxTimer        DrvTimer;       //��ʱ����������.
	
    STRUCT_RAMP     ramp;
    STRUCT_SYSTEM   system;
    STRUCT_CONFIG   config;
    
    void    OS_LoadTimer(void);     //��ʱ������Ƶ��
    
    void    OS_LRamp(void);         //ֱ�߼Ӽ���.
    void    OS_SRamp(void);         //S���߼Ӽ���.
    void    OS_NRamp(void);         //�޼Ӽ���.
    
    void    OS_Direction(void);         //����˶��������.
    Uchar   LSW_OS_LimitSwitch(void);   //��λ���غ���.
    Uchar   SoftLSW_OS_Limit(void);     //�����λ
    void    OS_ClrTimerShareFlg();      //���㹲��ʱ����־λ
};



#endif




