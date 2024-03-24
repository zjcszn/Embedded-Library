#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "dhcpd.h"
#include "webserver.h"
#include "airkiss_demo.h"
#include <cctype>

/**
 *������汾˵��
 *
 *V1.0	��ʼ�汾
 *V1.1  �޸���wep���ܷ�ʽ��֧��
 *V1.2  �޸���tkip���ܷ�ʽ��֧��,�����󲿷ֲ��ֱ��뾯��
 *V1.3  �޸���tkip/aes��ϼ��ܷ�ʽ��֧�֣������Ѿ���֧�����м��ܷ�ʽ
 *V1.4  �޸�����ʱ����Ƶ�����Ե�����
 *V1.5  �޸�airkiss�����޷�������������
 *V1.6  �޸�aes_decipher�����ڴ�й¶������
 *V1.7  �޸��ڵ���wifi_disconnect֮���޷��ٴν������ģʽ������(�ο�wifi_set_promisc)
 */


#define VIRSION		"V1.7"

wait_event_t command_event = 0;
char command = 0;

extern timer_t *led4_timer, *led5_timer;

#define help_string   \
"���Թ���:\r\n"\
"	0:������Ϣ\r\n"\
"	1:ɨ��\r\n"\
"	2:����WIFI\r\n"\
"	3:�ر�WIFI����\r\n"\
"	4::����AP\r\n"\
"	5:����TCP\r\n"\
"	6:�ر�TCP����\r\n"\
"	7:send����\r\n"\
"	8:����TCP������\r\n"\
"	9:�ر�TCP������\r\n"\
"	a:������Ƶ����\r\n"\
"	b:�ر���Ƶ����\r\n"\
"	c:dns����\r\n"\
"	d:�Զ���ȡIP��ַ\r\n"\
"	e:��ӡ�ڴ���Ϣ\r\n"\
"	f:��ȡwifi������Ϣ\r\n"\
"	g:����UDP\r\n"\
"	h:����ಥ��\r\n"\
"	i:����UDP������\r\n"\
"	j:sendto����\r\n"\
"	k:�л���Ƶģʽ(mp3/pcm)\r\n"\
"	l:��������\r\n"\
"	m:DEBUG���\r\n"\
"	n:\r\n"\
"	o:\r\n"\
"	p:airkiss+machtalk����\r\n"\
"	q:\r\n"\
"	r:ʹ��ʡ��ģʽ\r\n"\
"	s:�ر�ʡ��ģʽ\r\n"\
"	t:monitor���\r\n"\
"	u:����TCPȫ�ٷ���\r\n"\
"	v:����TCPȫ�ٷ���\r\n"\
"	w:�ط����Կ���\r\n"\
"	x:����\r\n"\
"	y:����machtalkԶ�̿���\r\n"\
"	z:����airkiss����ģʽ\r\n"\
"	?:����\r\n"\
"���������������ѡ����ԵĹ���:"


void print_help()
{
	p_dbg(help_string);
}

extern test_create_adhoc(void);
void handle_cmd(char cmd)
{
	cmd = tolower(cmd);
	switch (cmd)
	{
		case '0':
			show_sys_info();
			break;
		case '1':
			test_scan();
			break;
		case '2':
			test_wifi_connect();
			break;
		case '3':
			test_wifi_disconnect();
			break;
		case '4':
			test_create_ap();
			break;
		case '5':
			test_tcp_link();
			break;
		case '6':
			test_tcp_unlink();
			break;
		case '7':
			test_send("this is test data");
			break;
		case '8':
			test_tcp_server();
			break;
		case '9':
			test_close_tcp_server();
			break;
#if SUPPORT_AUDIO			
		case 'a':
			test_open_audio();
			break;
		case 'b':
			test_close_audio();
			break;
#endif			
		case 'c':
			test_dns("www.baidu.com");
			break;
		case 'd':
			test_auto_get_ip();
			break;
		case 'e':
			mem_slide_check(1);
			break;
		case 'f':
			test_wifi_get_stats();
			break;
		case 'g':
			test_udp_link();
			break;
		case 'h':
			test_multicast_join();
			break;
		case 'i':
			test_udp_server();
			break;
		case 'j':
			test_sendto("this is test data");
			break;
#if SUPPORT_AUDIO
		case 'k':
			switch_audio_mode();
			break;
		case 'l':
			if(audio_cfg.volume > 0)
				audio_cfg.volume -= 10;
			else
				audio_cfg.volume = 100;
			p_dbg("��ǰ����:%d", audio_cfg.volume);
			break;
#endif
		case 'm':
			switch_dbg();
			break;
		case 'n':
			test_wifi_connect1();
			break;
		case 'o':
			wifi_set_promisc(TRUE);
			break;
		case 'p':
			custom_test_demo();
			break;
		case 'q':
			break;
		case 'r':
			test_power_save_enable();
			break;
		case 's':
			test_power_save_disable();
			break;
		case 't':
			monitor_switch();
			break;
		case 'u':
			test_full_speed_send();
			break;
		case 'v':
			test_full_speed_send_stop();
			break;
		case 'w':
			switch_loopback_test();
			break;
		case 'x':
			soft_reset();
			break;
		case 'y':
#if MACHTALK_ENABLE
			test_connect_to_machtalk_server();
#endif
			break;
		case 'z':
			start_airkiss();
			break;
		case '?':
			print_help();
			break;
		default:
			p_err("unkown cmd");
			break;
	}
}

void main_thread(void *pdata)
{
	int ret;
#ifdef DEBUG
	RCC_ClocksTypeDef RCC_ClocksStatus;
#endif
	
	driver_misc_init(); //��ʼ��һЩ����(�������)
	usr_gpio_init(); //��ʼ��GPIO
	LED6_ON; //����LED6

	OSStatInit(); //��ʼ��UCOS״̬

	uart1_init(); //��ʼ������1

	//��ӡMCU����ʱ��
#ifdef DEBUG
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	p_dbg("SYSCLK_Frequency:%d,HCLK_Frequency:%d,PCLK1_Frequency:%d,PCLK2_Frequency:%d,ADCCLK_Frequency:%d\n", 
		RCC_ClocksStatus.SYSCLK_Frequency, 
		RCC_ClocksStatus.HCLK_Frequency, 
		RCC_ClocksStatus.PCLK1_Frequency, 
		RCC_ClocksStatus.PCLK2_Frequency, 
		0);
#endif

	m25p80_init(); //��ʼ��SPI-flash
#if SUPPORT_AUDIO
	audio_dev_init(); //��ʼ����Ƶ���������ݽṹ
#endif
	init_work_thread();	//��ʼ�������߳�

    	misc_init(); //��ʼ��һЩ����(app���)

	ret = SD_Init();    //��ʼ��SDIO�豸
		
	if (!ret){
		ret = init_wifi();//��ʼ��WIFIоƬ
	}

	if(ret){//������ʼ��ʧ��,û������������
		p_err("wifi card faild");  
       		mod_timer(led4_timer, 100); //LED4��ʼ��˸
		while(1)sleep(1000);    
	}

   	 init_monitor(); //��ʼ��monitorģ��,������init_wifi֮�����

	 load_cfg();

	init_lwip(); //��ʼ��lwipЭ��ջ

	init_udhcpd(); //��ʼ��dhcp������

	enable_dhcp_server(); // ����dhcp������,���������staģʽ,���Բ�����dhcpserver

	airkiss_demo_init(); //Ԥ�ȳ�ʼ��airkiss

	command_event = init_event(); //��ʼ��һ���ȴ��¼�

	p_dbg("����汾:%s", VIRSION);
	p_dbg("����ʱ��%d.%d S", os_time_get() / 1000, os_time_get() % 1000);
	
/*
 *��������������Ԥ��Ĺ���
 *����web������, �˿ں�:80
 *����TCP���������˿ں�:4800
 *���ӵ�·������·�������ƺ������ͨ��airkiss����
 *
*/
#if	SUPPORT_WEBSERVER
	web_server_init();
#endif
	test_tcp_server();
	//test_wifi_connect();
	
	print_help();

	LED6_OFF; 

	while (1)
	{
		//�ȴ��û�����
		wait_event(command_event);

		//ִ������
		handle_cmd(command);
	}
}

int main(void)
{

	OSInit();

	_mem_init(); //��ʼ���ڴ����

	thread_create(main_thread, 0, TASK_MAIN_PRIO, 0, TASK_MAIN_STACK_SIZE, "main_thread");

	OSStart();
	return 0;
}
