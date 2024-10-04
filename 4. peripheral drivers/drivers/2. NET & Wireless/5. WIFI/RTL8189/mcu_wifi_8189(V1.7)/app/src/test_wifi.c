#define DEBUG
#include "debug.h"
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "airkiss_demo.h"

#include "dhcpd.h"

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_byteorder.h>

#include <hal_intf.h>
#include <lwip/netif.h>

int wifi_get_stats(uint8_t *mac, struct station_info *sinfo);
int wifi_scan(void (*scan_result_callback)(BSS_TABLE_SIMPLE *Tab), 
		char *essid);

void print_scan_result(struct wlan_network *dat)
{
	char *cipher = "";
	char *mode;
	
	char essid_name[33];

	if(dat->BcnInfo.encryp_protocol == ENCRYP_PROTOCOL_OPENSYS)
		cipher = "NONE";
	else
	{
		if(dat->BcnInfo.encryp_protocol == ENCRYP_PROTOCOL_WEP)
			cipher = "WEP";
		if(dat->BcnInfo.encryp_protocol == ENCRYP_PROTOCOL_WPA)
			cipher = "WPA";
		if(dat->BcnInfo.encryp_protocol == ENCRYP_PROTOCOL_WPA2)
			cipher = "WPA2";
	}	

	if(dat->network.InfrastructureMode == Ndis802_11IBSS)
		mode = "AD-HOC";
	else if(dat->network.InfrastructureMode == Ndis802_11Infrastructure)
		mode = "AP";
	else
		mode = "unkown";
	
	memcpy(essid_name, dat->network.Ssid.Ssid,32);
	essid_name[32] = 0;
	
	p_dbg("	�ź�ǿ��:%ddbm",dat->network.Rssi);
	p_dbg("	����ģʽ:%s", cipher);
	p_dbg("	ģʽ:%s", mode);	
	p_dbg("	ssid:%s",essid_name);
	dump_hex("	bssid",dat->network.MacAddress, 6);
}


void scan_done_callback(BSS_TABLE_SIMPLE *Tab)
{
	int i = 0;
	if(Tab)
	{
		BSS_ENTRY_SIMPLE *p_bss_entry = Tab->BssEntry;
		if (p_bss_entry)
		{
			do
			{
				p_dbg("\r\n%d: %s", i++, p_bss_entry->res_data.essid);
				
				p_dbg(" mode:%d", p_bss_entry->res_data.mode);
				p_dbg(" auth:%d", p_bss_entry->res_data.auth);
				p_dbg(" rssi:%d", p_bss_entry->res_data.rssi);
				p_dbg(" channel:%d", p_bss_entry->res_data.channel);
				dump_hex("	mac", p_bss_entry->res_data.bssid, 6);
				p_bss_entry = p_bss_entry->next;
			}while (p_bss_entry);
		}

	}
}

/*
*����ɨ��,����'1'
*/
void test_scan()
{
	p_dbg_enter;
	wifi_scan(scan_done_callback, "");
	p_dbg_exit;
}

/**
 *����WIFI����,����'2'
 *���ӵ�����Ϊ"LCK����·����������ģʽ��Ƶ���Զ���Ӧ
 *���볤����WPA��WPA2ģʽ��8 <= len <= 64;��WEPģʽ�±���Ϊ5��13
 */
void test_wifi_connect()
{
	char *essid = save_cfg.link.essid;
	char *password = save_cfg.link.key;
	int connect_timeout = 50;
	p_dbg_enter;

	p_dbg("���ӵ�:%s, ����:%s", essid, password);

	if (is_wifi_connected())
		wifi_disconnect();

	wifi_set_mode(MODE_STATION);

	wifi_scan(NULL, essid);

	sleep(2000);

	wifi_connect(essid, password);

	while(connect_timeout--)
	{
		if (is_wifi_connected())
		{
			break;
		}
		sleep(100);
	}
	
	if (is_wifi_connected())
	{
		p_dbg("wifi connect ok");
	}
	p_dbg_exit;

}

void test_wifi_connect1()
{
	char *essid = /*save_cfg.link.essid*/"note";
	char *password = /*save_cfg.link.key*/"ckxr1314";
	int connect_timeout = 50;
	p_dbg_enter;

	p_dbg("���ӵ�:%s, ����:%s", essid, password);

	if (is_wifi_connected())
		wifi_disconnect();

	wifi_set_mode(MODE_STATION);

	wifi_scan(NULL, essid);

	sleep(2000);

	wifi_connect(essid, password);

	while(connect_timeout--)
	{
		if (is_wifi_connected())
		{
			break;
		}
		sleep(100);
	}
	
	if (is_wifi_connected())
	{
		p_dbg("wifi connect ok");
	}
	p_dbg_exit;

}

/*
 *���ԶϿ�WIFI����,����'3'
 *
 */
void test_wifi_disconnect()
{
	p_dbg_enter;
	wifi_disconnect();
	p_dbg_exit;
}



/*
 *����AP,����'4'��AP������δ���ƣ�
 *
 */
void test_create_ap()
{
	char *essid = "xrf_ap";
	char *password = "12345678";
	p_dbg_enter;
	p_dbg("����ap:%s, ����:%s", essid, password);

	wifi_set_mode(MODE_AP);

	wifi_ap_cfg(essid, password, KEY_WPA2, 6, 4);

	p_dbg_exit;
}


/*
 *���Ի�ȡWIFI������Ϣ,������ж���ͻ�������,������Ҫָ�����ַ
 *�����test_get_station_list����ȡ���һ��mac��ַ��Ϊ����
 *����ִ�д˲���֮ǰ����ִ��test_get_station_list
 *,����'f'
 */
uint8_t sta_mac[8] = {0,0,0,0,0,0};
void test_wifi_get_stats()
{
	int ret;
	struct station_info sinfo;
	
	ret = wifi_get_stats(sta_mac, &sinfo);

	if(ret == 0)
		p_dbg("wifi stats, rssi:%d", sinfo.signal);
	else
		p_err("test_wifi_get_stats err");
}

void test_power_save_enable()
{
	p_dbg("enter power save");
	wifi_power_cfg(1);
}

void test_power_save_disable()
{
	p_dbg("exit power save");
	wifi_power_cfg(0);
}

/*
*�ͻ����Է�����������������
*1. ����airkiss�ȴ��û�����·����ssid������(ͨ��airkissdemo����΢��)
*2. ·����ssid���������óɹ���·�����ɹ����ӣ����Զ���ȡIP��ַ
*3. ����machtalkԶ�̷�����
*4. �û�ͨ��machtalk_remote_contrl���ͨ��Զ�̿��ƿ������led
*/
void custom_test_demo()
{
	int ret;
	ret = start_airkiss();
	if(ret || !is_wifi_connected()){
		p_err("airkiss config faild");
		return;
	}
	//��ʱWIFI�Ѿ������ϣ� ��ʼ��ȡ��ip��ַ
	test_auto_get_ip();
	if (!p_netif->ip_addr.addr)
	if(ret){
		p_err("auto get ip faild");
		return;
	}

	//��ʼmachtalk����
	test_connect_to_machtalk_server();
}

