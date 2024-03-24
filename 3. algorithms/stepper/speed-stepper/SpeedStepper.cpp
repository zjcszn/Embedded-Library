/*******************************************************************************
                        �ൺ����������Ƭ�����޹�˾
                            ��Ȩ���� @ 2018

�������������.��Ҫ���ڼӼ���Ƶ�����ٶ���Ӧ��.���Է���Ŀ��ƼӼ��ٵļ��ٶ�.
�Ӽ��ٵļ��ٶȿ��Բ�ͬ,��������ʱ�ı�.

������
2018.04.08
*******************************************************************************/
#include  "SpeedStepper.h"



/*******************************************************************************
�� ��: SpeedStepper(void)
�� ��: ���캯��.��ɱ����ĳ�ʼ��.
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
SpeedStepper::SpeedStepper(void)
{
    hardware.PinPls     = null;
    hardware.PinDir     = null;
    hardware.PlsPerCir  = 400;
    hardware.StopLevel  = LOW;
    
    ramp.RampUpStep     = 1.0;
    ramp.RampDnStep     = 1.0;
    
    system.CurFrq       = 0;            //ϵͳ״̬�ṹ��.
    system.CurDir       = DIR_ZHENG;
    system.location     = 0;
    system.locationHi   = 0;
    system.State        = MOTOR_STOP;
    
    system.EnMaxLocationLimit = false;      //��������λ
    system.MaxLocation        = 100000000;
    system.EnMinLocationLimit = false;
    system.MinLocation        = -100000000; //�����С��λ
    
    config.SetFrq       = 0;            //����Ƶ��ֵ
    config.DirOpp       = false;        //����Ƿ���
    config.PlsDelayTime = 5;            //������ʱʱ��.Ĭ��5uS.
}

/*******************************************************************************
�� ��: void begin(  TIM_TypeDef* UseTimer,
                    Uint  PlusPerCir, 
                    Uchar DrvPls, Uchar DrvDir,
                    Ulong PlsMod, Ulong DirMod,
                    Uchar PlsStopLvl )
�� ��: ��ʼ��������������ĸ��ֲ���.
�� ��: UseTimer   -- �����ò������ʹ�õĶ�ʱ������.
       PlusPerCir -- �������ת��һȦ����Ҫ����������.
       DrvPls     -- ��������.
       DrvDir     -- ��������.
       PlsMod     -- �������Ŷ˿�ģʽ.
       DirMod     -- �������Ŷ˿�ģʽ.
       PlsStopLvl -- ��������ֹͣʱ��ƽ.
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::begin(   TIM_TypeDef* UseTimer,
                            TIM_TypeDef* RampTimer,
                            Uint  PlusPerCir, 
                            Uchar DrvPls,
                            Uchar DrvDir,
                            Uchar PlsStopLvl )
{
    DrvTimer.begin(UseTimer);           //�������ʹ�õĶ�ʱ��.
    DrvTimer.set(100);                  //���ö�ʱ�����ж�Ƶ��.
    DrvTimer.close();                   //�رն�ʱ��
    
    if( RampTimer!=null )               //�Ӽ��ٶ�ʱ��
    {
        RmpTimer.begin(RampTimer);
        RmpTimer.set(100);              //�Ӽ��ٶ�ʱ���̶�ÿ100ms�ж�һ��
        RmpTimer.open();                //�������Ӽ��ٷǳ��ǳ���(���ٶȴ�)
    }                                   //���ԼӴ��ж�Ƶ��
    
    hardware.PlsPerCir = PlusPerCir;    //�������ת��һȦ����Ҫ����������.
    
    hardware.PinPls    = DrvPls;        //��������+��������.
    hardware.PinDir    = DrvDir;
    
    pinMode(hardware.PinPls, OUTPUT);   //��������.
    pinMode(hardware.PinDir, OUTPUT);   //��������.
    
    OS_Direction();                     //�������.
    hardware.StopLevel = PlsStopLvl;    //ͣ��ʱ�����ƽ.
    
    config.MinSpeed = 0.1;              //Ĭ�����ת��0.1RPM
    config.MinFrq   = (0.1)*(float)(hardware.PlsPerCir)/60.00;
    
    stop();                             //�����ʼ����ʱ��ͣ��.
}

/*******************************************************************************
�� ��: Uchar CurDir(void)
�� ��: ��ȡ��ǰ������˶�����.
�� ��: None
�� ��: DIR_ZHENG/DIR_FAN

������
2018.04.08
*******************************************************************************/
Uchar SpeedStepper::CurDir(void)
{
    return (system.CurDir);
}

/*******************************************************************************
�� ��: void DirOpp(Uchar _Cmd)
�� ��: �˶������Ƿ�ȡ��.
�� ��: _Cmd--�˶������Ƿ�ȡ��.false--��ȡ��;true--ȡ��.
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::DirOpp(Uchar _Cmd)
{
    if(_Cmd)
    {
        config.DirOpp = true;
    }
    else
    {
        config.DirOpp = false;
    }
}

/*******************************************************************************
�� ��: void MinSpeedpeed(float _RPM)
�� ��: ���ò�������������ת�ٶ�.
�� ��: myRPM -- �������ת���ٶ�.��λRPM.
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::MinSpeed(float _RPM)
{
    if(_RPM<0)      //���õ�ת��С��0.������.
    {               //���Ե�����
        return;
    }
    
    config.MinSpeed = _RPM;
    config.MinFrq   = (_RPM)*(float)(hardware.PlsPerCir)/60.00;
}

/*******************************************************************************
�� ��: void speed(float myRPM)
�� ��: ���ò��������ת��.
�� ��: myRPM -- �������ת���ٶ�.��λRPM.
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::speed(float _RPM)
{
    if(_RPM<0)      //���õ�ת��С��0.������.
    {               //���Ե�����.�������Ƶ������ͣ��.
        return;
    }
    
    config.SetFrq = (_RPM)*(float)(hardware.PlsPerCir)/60.00;
    
//  Serial.print("SetFrq:");
//  Serial.println(config.SetFrq);
}

/*******************************************************************************
�� ��: void run(void)
�� ��: �����ʼ����.
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::run(void)
{
    if( state()!=MOTOR_STOP )       //�������������.
    {
    //  Serial.println("MOTOR Running!");
        return;
    }
    
    if( config.SetFrq==0 )          //�����ٶ�����.����Ҫ�������.
    {
    //  Serial.println("SetFrq is Zero!");
        return;
    }
    
    // SpeedStepper���ܹر�б�¹���
    // ��ǰ����϶���ֹͣ��
    // ���״̬: ������
    system.State  = MOTOR_UP;        
    system.CurFrq = ramp.RampUpStep;
    
    if(system.CurFrq>config.SetFrq)
    {
        system.CurFrq=config.SetFrq;
    }  
    
    //������е���СƵ��
    float MinFrq = (config.MinSpeed)*(float)(hardware.PlsPerCir)/60.00;
    if( system.CurFrq<MinFrq )
    {
        system.CurFrq = MinFrq;
    }
    
    OS_LoadTimer();                 //���¼��ض�ʱ��.
    DrvTimer.SetCounter(0xFFFF);    //V1.1�޸�.�������������ʱ��,��ʱʱ�����.
                                    //ԭ����TimerԤ��Ƶ���õڶ����жϲ���Ч.
                                    //��һ���ж�ʱ�����.6������.����ͻ��ӳ�6��������.
                                    //����ǿ�ȵ�һ���ж���������.
    DrvTimer.open();                //������ʱ������.
}

/*******************************************************************************
�� ��: float CurSpeed(void)
       float CurFrq(void)
�� ��: ��ȡ��ǰת��/����Ƶ��
�� ��: None
�� ��: RPM/Hz

������
2018.04.08
*******************************************************************************/
float SpeedStepper::CurSpeed(void)
{
    if( hardware.PlsPerCir<=0 ) //���ת1Ȧ��������
    {
        hardware.PlsPerCir = 200;
    }
    
    float RData = (system.CurFrq) * 60.0 / (float)(hardware.PlsPerCir);
    
    return RData;               //��λ:RPM
}

float SpeedStepper::CurFrq(void)
{
    return system.CurFrq;       //��λ:Hz
}

/*******************************************************************************
�� ��: void PlsDelay(Uint Dly)
�� ��: ������ʱʱ��.
�� ��: Dly--��ʱʱ��.��λ:΢��.
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::PlsDelay(Uint Dly)
{
    config.PlsDelayTime = Dly;
}

/*******************************************************************************
�� ��: Uchar dir(Uchar RunDir, long RunSteps, Uchar _Cmd)
�� ��: �޸Ĳ���������˶�����
�� ��: RunDir : �������ת������. 0/1.
       _Cmd   : �Ƿ�ǿ�Ƹ��µ�����з���
�� ��: ִ�гɹ�/û��ִ��

������
2018.04.08
*******************************************************************************/
Uchar SpeedStepper::dir(Uchar RunDir,Uchar _Cmd)
{
    if( (system.CurDir!=RunDir)&&(_Cmd==false) )
    {
        return false;
    }
    
    system.CurDir = RunDir;     //��¼�˶�����.
    OS_Direction();             //���˶��������������.  
    
    return true;
}

Uchar SpeedStepper::dir(Uchar RunDir)
{
    dir(RunDir,true);
}

/*******************************************************************************
�� ��: void Stop(void)
�� ��: ����ֹͣ�������������.�˺���������ֹͣ���,��û�м���Ч��.
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::stop(void)
{
    config.SetFrq = 0;
    system.CurFrq = 0;
    system.State  = MOTOR_STOP;
    
    DrvTimer.close();
}

/*******************************************************************************
�� ��: Uchar get(void)
�� ��: ��ȡ��ǰ���������״̬.
�� ��: None
�� ��: MOTOR_STOP/MOTOR_UP/MOTOR_RUN/MOTOR_DOWN

������
2018.04.08
*******************************************************************************/
Uchar SpeedStepper::state(void)
{
    return (system.State);
}

/*******************************************************************************
�� ��: long location(void)
�� ��: ��ȡȫ��λ��/����ȫ��λ��.
�� ��: ����.
�� ��: ����.

������
2018.04.08
*******************************************************************************/
long SpeedStepper::location(void)
{
    return (system.location);
}

void SpeedStepper::location(long _location)
{
    system.location = _location;
}

long SpeedStepper::locationHi(void)
{
    return (system.locationHi);
}

void SpeedStepper::locationHi(long _locationHi)
{
    system.locationHi = _locationHi;
}

/*******************************************************************************
�� ��: void MaxLocation(Uchar _Cmd,long Value)
�� ��: ���õ���������λ���ֵ
�� ��: _Cmd:�Ƿ����������λ;Value:�����λֵ.
�� ��: None

������
2018.11.21
*******************************************************************************/
void SpeedStepper::MaxLocation(Uchar _Cmd,long Value)
{
    if(_Cmd)
    {
        system.EnMaxLocationLimit = true;
        system.MaxLocation        = Value;
    }
    else
    {
        system.EnMaxLocationLimit = false;
        system.MaxLocation        = Value;
    }
}

/*******************************************************************************
�� ��: void MinLocation(Uchar _Cmd,long Value)
�� ��: ���õ���������λ��Сֵ
�� ��: _Cmd:�Ƿ�������С��λ;Value:��С��λֵ.
�� ��: None

������
2018.11.21
*******************************************************************************/
void SpeedStepper::MinLocation(Uchar _Cmd,long Value)
{
    if(_Cmd)
    {
        system.EnMinLocationLimit = true;
        system.MinLocation        = Value;
    }
    else
    {
        system.EnMinLocationLimit = false;
        system.MinLocation        = Value;
    }
}

/*******************************************************************************
�� ��: void OS_LoadTimer(void)
�� ��: ���ض�ʱ������Ƶ��
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::OS_LoadTimer(void)
{
    DrvTimer.set(system.CurFrq);
}

/*******************************************************************************
�� ��: void OS_MotorStps(void)
       void OS_MotorEnd(void)
�� ��: ������������������.������صĶ˿�.
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
inline void SpeedStepper::OS_MotorStps(void)
{
    digitalWrite(hardware.PinPls,!hardware.StopLevel);
    delayMicroseconds(config.PlsDelayTime);
}

inline void SpeedStepper::OS_MotorEnd(void)
{
    digitalWrite(hardware.PinPls,hardware.StopLevel);
}

/*******************************************************************************
�� ��: void OS_location()
�� ��: ��¼�������ȫ��λ��.
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::OS_location(void)
{
    if(system.CurDir==DIR_ZHENG)        
    {
        system.location++;              //ȫ��λ������
        
        if(system.location>2000000000L)
        {
            system.location = 0;
            system.locationHi++;
        }
        
        if(system.EnMaxLocationLimit)   //ʹ�������ֵ��λ
        {
            if(system.location>=system.MaxLocation)
            {
                speed(0);
            }
        }
    }
    else
    {
        system.location--;              //ȫ��λ������
        
        if(system.location<-2000000000L)
        {
            system.location = 0;
            system.locationHi--;
        }
        
        if(system.EnMinLocationLimit)   //ʹ������Сֵ��λ
        {
            if(system.location<=system.MinLocation)
            {
                speed(0);
            }
        }
    }
}

/*******************************************************************************
�� ��: void ARamp(long _Accel)
�� ��: ����ϵͳ���ٶ�����
�� ��: _AccelUp:����ʱ��ļ��ٶ�.��λRPM/S.
       _AccelDn:����ʱ��ļ��ٶ�.
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::ARamp(float _AccelUp,float _AccelDn)
{
    float UpFrq = (_AccelUp)*(float)(hardware.PlsPerCir)/60.00;
    float DnFrq = (_AccelDn)*(float)(hardware.PlsPerCir)/60.00;
    
    float UpStep= MaxRounding(UpFrq/100.0,4);
    float DnStep= MaxRounding(DnFrq/100.0,4);
    
    if(UpStep<=0.1)
    {
        UpStep=0.1;
    }
    if(DnStep<=0.1)
    {
        DnStep=0.1;
    }
    
    ramp.RampUpStep = UpStep;
    ramp.RampDnStep = DnStep;
}

void SpeedStepper::ARamp(float _Accel)
{
    ARamp(_Accel,_Accel);
}

/*******************************************************************************
�� ��: void OS_Direction(void)
�� ��: ����˶��������.
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::OS_Direction(void)
{
    if(config.DirOpp)
    {
        digitalWrite(hardware.PinDir, !system.CurDir);
    }
    else
    {
        digitalWrite(hardware.PinDir, system.CurDir);
    }
    
    delayMicroseconds(10);  //��΢��ʱ �ȴ��������ŵ�ƽ�ȶ�
}

/*******************************************************************************
�� ��: void OS_HiLRamp(void)
�� ��: ֱ�߼Ӽ��ٺ���.���ٺ���.������ÿ10��������1��.
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::OS_HiLRamp(void)
{
    if(system.State==MOTOR_STOP)            //�������ֹͣ״̬.
    {                                       //ֱ���˳�.2018.05.01
        return;
    }
    
    if( system.CurFrq<config.SetFrq )       //��Ҫ����
    {
        system.CurFrq += ramp.RampUpStep;
        
        if( system.CurFrq<config.MinFrq )    //��������ٶ���MinFrq
        {
            system.CurFrq = config.MinFrq;
        }
        if(system.CurFrq>config.SetFrq)
        {
            system.CurFrq=config.SetFrq;
        }
        
        OS_LoadTimer();                     //���¼��ض�ʱ��.
        system.State = MOTOR_UP;            //���״̬:������.
    }
    else if( system.CurFrq>config.SetFrq )  //��Ҫ����
    {
        system.CurFrq -= ramp.RampDnStep;
        
        if(system.CurFrq<config.SetFrq)
        {
            system.CurFrq=config.SetFrq;
        }
        if(system.CurFrq<config.MinFrq)     //��ǰ�ٶȵ�������ٶ�
        {
            OS_LoadTimer();                 //���¼��ض�ʱ��.   
            DrvTimer.close();               //������ʱ������.
            system.State = MOTOR_STOP;      //��־λ.ֹͣ.
        }
        
        OS_LoadTimer();                     //���¼��ض�ʱ��. 
        system.State = MOTOR_DOWN;          //���״̬:������.
    }
    else
    {
        if(system.CurFrq==0)                //��ǰ״̬,ֹͣ.
        {
            OS_LoadTimer();                 //���¼��ض�ʱ��.   
            DrvTimer.close();               //������ʱ������.
            system.State = MOTOR_STOP;      //��־λ.ֹͣ.
        }
        else
        {
            OS_LoadTimer();                 //���¼��ض�ʱ��. 
            system.State = MOTOR_RUN;       //���״̬:������.
        }
    }
}

/*******************************************************************************
�� ��: void TISR(void)
�� ��: �Ӽ����жϺ���.�̶����ڵ��øú���.�������Ƶ���Ӽ���.100Hz
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::TISR(void)
{
    OS_HiLRamp();
}

/*******************************************************************************
�� ��: void SpeedStepper::System_MotorISR(void)
�� ��: ����������ɶ�ʱ���жϵ��õĺ���.�������������ź�.
�� ��: None
�� ��: None

������
2018.04.08
*******************************************************************************/
void SpeedStepper::scan(void)
{
    OS_MotorStps();     //��ʼ��������ź�.
    OS_location();      //��¼ȫ��λ��.
    OS_MotorEnd();      //�ر��������.
}




