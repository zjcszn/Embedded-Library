/*
 * 
 *
 */
#define DEBUG
#include "drivers.h"
#include "app.h"
#include "api.h"

#include "lwip/pbuf.h"
#include "lwip/sys.h"

#include "moal_sdio.h"

#include <osdep_service.h>
#include <drv_types.h>
#include <xmit_osdep.h>
#include <recv_osdep.h>
#include <hal_intf.h>
#include <rtw_ioctl.h>
#include <rtw_version.h>
#include <rtw_ioctl_set.h>
#include <rtl8188e_hal.h>
#include <stdarg.h>
#include "wpa.h"
#include "sha1.h"
struct net_device *wifi_dev = NULL;

struct net_device *get_wifi_dev()
{
	return wifi_dev;
}

int is_hw_ok()
{
	return wifi_dev?1:0;
}

/**
 * @brief  ����wifiģʽ,ֻ���л�ģʽ,����ap������������
 * @param  type : MODE_AP, MODE_STATION, MODE_ADHOC
 * @retval ok: 0, err: -1
 */
int wifi_set_mode(int type)
{
	int ret = -1;
	int set_type;
//	struct	mlme_priv	*pmlmepriv;
	_adapter *padapter;

	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);
//	pmlmepriv = &(padapter->mlmepriv);
	
	if(type == MODE_AP){
		set_type = NL80211_IFTYPE_AP;
	}
	else if(type == MODE_STATION){
		set_type = NL80211_IFTYPE_STATION;
	}

	else if(type == MODE_ADHOC){
		set_type = NL80211_IFTYPE_ADHOC;
	}
	else{
		p_err("type %d not support", type);
		return -1;
	}

	if(wifi_dev == NULL){

		p_err("%s not support", __FUNCTION__);
		return -1;
	}

	if(wifi_dev->cfg80211_ops->change_virtual_intf){

		p_dbg("change_virtual_intf");
		//ap��staָ����ͬ��change_virtual_intf
		ret = wifi_dev->cfg80211_ops->change_virtual_intf(padapter->rtw_wdev->wiphy, wifi_dev, (enum nl80211_iftype)set_type, 0, 0);
	}else
		p_err("%s not support", __FUNCTION__);

	return ret;
}


/**
 * @brief  ����smart_link�ص�����������֮��ͻῪʼ����ÿһ������
 ��ʹ��smart_link��ʱ����Ҫ��0
 * @retval ok: 0, err: -1
 */
int wifi_set_smart_link_recv_hook(smart_link_recv_hook hook)
{
	int ret = -1;
	if(wifi_dev && wifi_dev->netdev_ops)
		ret = wifi_dev->netdev_ops->set_smart_link_recv_hook(wifi_dev, (void*)hook);

	return ret;
}

/**
 * @brief  ����wifiƵ��,for ap
 * @param  channel : 1 - 14
 * @retval ok: 0, err: -1
 */
int wifi_set_channel(int channel)
{
	_adapter *padapter;

	if(!wifi_dev)
		return -1;

	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);

	rtw_set_ch_cmd(padapter, channel, 0, 0, 0);

	return 0;
}


/**
 * @brief  ����Ϊ����ģʽ
 * @param  
 * @retval ok: 0, err: -1
 */
int wifi_set_promisc(bool enable)
{
	_adapter *padapter;
		
	if(!wifi_dev)
		return -1;

	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);

	if(enable){
		rtw_write32(padapter, REG_RCR, 0x7000600e);  
	}else{
		rtw_write32(padapter, REG_RCR, rtw_read32(padapter, REG_RCR)|RCR_CBSSID_DATA|RCR_CBSSID_BCN); 
	}
	rtw_write16(padapter, REG_RXFLTMAP2, 0xFFFF);//recv all data frame
	return -1;	
}

struct cfg80211_scan_request *new_scan_req(_adapter *padapter, char *ssid, u8_t ssid_len)
{
	
	struct wiphy *wiphy = padapter->rtw_wdev->wiphy;
	struct cfg80211_scan_request *creq = NULL;
	int i, n_channels = 0;
	enum ieee80211_band band;

	for (band = (enum ieee80211_band)0; band < IEEE80211_NUM_BANDS; band++) {
		if (wiphy->bands[band])
			n_channels += wiphy->bands[band]->n_channels;
	}

	p_dbg("_new_connect_scan_req,channels:%d,creq_size:%d\n",n_channels,sizeof(*creq) + sizeof(struct cfg80211_ssid) +
		       n_channels * sizeof(void *));
	creq = (struct cfg80211_scan_request *)mem_calloc(sizeof(*creq) + sizeof(struct cfg80211_ssid) +
		       n_channels * sizeof(void *),1);
	if (!creq)
		return NULL;
	/* SSIDs come after channels */
	creq->wiphy = wiphy;
	creq->ssids = (void *)&creq->channels[n_channels];
	creq->n_channels = n_channels;
	if(ssid_len)
		creq->n_ssids = 1;

	/* Scan all available channels */
	i = 0;
	for (band = (enum ieee80211_band)0; band < IEEE80211_NUM_BANDS; band++) {
		int j;

		if (!wiphy->bands[band])
			continue;

		for (j = 0; j < wiphy->bands[band]->n_channels; j++) {
			/* ignore disabled channels */
			if (wiphy->bands[band]->channels[j].flags &
						IEEE80211_CHAN_DISABLED)
				continue;

			creq->channels[i] = &wiphy->bands[band]->channels[j];
			i++;
		}
	}
	if (i) {
		/* Set real number of channels specified in creq->channels[] */
		creq->n_channels = i;

		/* Scan for the SSID we're going to connect to */
		memcpy(creq->ssids[0].ssid, ssid, ssid_len);
		creq->ssids[0].ssid_len = ssid_len;
	} else {
		/* No channels found... */
		mem_free(creq);
		creq = NULL;
	}

	return creq;
	
}


/**
 * @brief  ɨ�踽����AP,usr_scan_result_callback�������Զ����һ���ص��ӿ�
 *		ÿɨ�赽һ��ap�ͻ���wlan_ret_802_11_scan->handle_user_scan_result����һ��,�����scan_result_data����һЩ����
 *		��ap��Ϣ,һ���Ѿ�����,���Ҫ����ϸ��ap����Ϣ��ֱ����handle_user_scan_result����ȡ
 * @param
 * @retval ok: 0, err: -1
 */
extern void (*usr_scan_result_callback)(BSS_TABLE_SIMPLE *Tab);

int wifi_scan(void (*scan_result_callback)(BSS_TABLE_SIMPLE *Tab), 
		char *essid)
{

	_adapter *padapter;
	struct cfg80211_scan_request *scan_request;
	
	if(!wifi_dev)
		return -1;
    
    padapter = (_adapter *)rtw_netdev_priv(wifi_dev);

	usr_scan_result_callback = scan_result_callback;
    rtw_free_network_queue(padapter, 1);
	scan_request = new_scan_req(padapter, essid, strlen(essid));

	
    if(strlen(essid))
        rtw_set_scan_mode(padapter, SCAN_ACTIVE);
    else
        rtw_set_scan_mode(padapter, SCAN_PASSIVE);
	
	BssTabRstSimple(&padapter->mlmepriv.ScanTabSimple);
	wifi_dev->cfg80211_ops->scan(padapter->rtw_wdev->wiphy, wifi_dev, scan_request);

	//scan���첽ʵ�ֵ�,���������������˳�
	//ɨ����ɺ��������Զ�����cfg80211_scan_done����

	return 0;
}


/**
 * @brief  wifi �Ƿ��Ѿ�����
 *
 */
int is_wifi_connected()
{
	int ret = (wpa_sm_get_state() == WPA_COMPLETED);
	return ret;
}

int wifi_add_key(u8 key_index, bool pairwise, const u8 *mac_addr,
			   struct key_params *params)
{
	_adapter *padapter;
	int ret = -1;
	p_dbg_enter;

	if(!wifi_dev)
		return -1;

	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);
	
	if(wifi_dev->cfg80211_ops->add_key)
		ret = wifi_dev->cfg80211_ops->add_key(padapter->rtw_wdev->wiphy, 
			wifi_dev,
			key_index, 
			pairwise, 
			mac_addr,
			params);
	return ret;
}


ULONG MakeOutgoingFrame(
	OUT UCHAR *Buffer, 
	OUT ULONG *FrameLen, ...) 
{
	UCHAR   *p;
	int 	leng;
	ULONG	TotLeng;
	va_list Args;

	/* calculates the total length*/
	TotLeng = 0;
	va_start(Args, FrameLen);
	do 
	{
		leng = va_arg(Args, int);
		if (leng == -1) 
		{
			break;
		}
		p = va_arg(Args, void*);
		memmove(&Buffer[TotLeng], p, leng);
		TotLeng = TotLeng + leng;
	} while(TRUE);

	va_end(Args); /* clean up */
	*FrameLen = TotLeng;
	return TotLeng;
}


/* 2-byte Frame control field */
struct FRAME_CONTROL{
  USHORT Ver:2;		/* Protocol version */
	USHORT Type:2;		/* MSDU type */
	USHORT SubType:4;	/* MSDU subtype */
	USHORT ToDs:1;		/* To DS indication */
	USHORT FrDs:1;		/* From DS indication */
	USHORT MoreFrag:1;	/* More fragment bit */
	USHORT Retry:1;		/* Retry status bit */
	USHORT PwrMgmt:1;	/* Power management bit */
	USHORT MoreData:1;	/* More data bit */
	USHORT Wep:1;		/* Wep data */
	USHORT Order:1;		/* Strict order expected */
} __packed;


struct HEADER_802_11 {
  struct FRAME_CONTROL   FC;
  USHORT          Duration;
  UCHAR           Addr1[6];
  UCHAR           Addr2[6];
	UCHAR			Addr3[6];
	USHORT			Frag:4;
	USHORT			Sequence:12;
}__packed;

#define BTYPE_MGMT                  0
#define BTYPE_CNTL                  1
#define BTYPE_DATA                  2

VOID MgtMacHeaderInit(
	struct HEADER_802_11 *pHdr80211, 
	UCHAR SubType, 
	UCHAR ToDs, 
	PUCHAR pDA, 
	PUCHAR pBssid) 
{
	memset(pHdr80211, 0, sizeof(struct HEADER_802_11));
	
	pHdr80211->FC.Type = BTYPE_MGMT;
	pHdr80211->FC.SubType = SubType;
/*	if (SubType == SUBTYPE_ACK)	 sample, no use, it will conflict with ACTION frame sub type*/
/*		pHdr80211->FC.Type = BTYPE_CNTL;*/
	pHdr80211->FC.ToDs = ToDs;
	memcpy(pHdr80211->Addr1, pDA, 6);
//#ifdef CONFIG_AP_SUPPORT
//	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	memcpy(pHdr80211->Addr2, pBssid, 6);
//#endif /* CONFIG_AP_SUPPORT */
	memcpy(pHdr80211->Addr3, pBssid, 6);
}

#define TIMESTAMP_LEN                   8
#define SUBTYPE_ASSOC_REQ           0
#define SUBTYPE_ASSOC_RSP           1
#define SUBTYPE_REASSOC_REQ         2
#define SUBTYPE_REASSOC_RSP         3
#define SUBTYPE_PROBE_REQ           4
#define SUBTYPE_PROBE_RSP           5
#define SUBTYPE_BEACON              8
#define SUBTYPE_ATIM                9
#define SUBTYPE_DISASSOC            10
#define SUBTYPE_AUTH                11
#define SUBTYPE_DEAUTH              12
#define SUBTYPE_ACTION              13
#define SUBTYPE_ACTION_NO_ACK              14
const UCHAR BROADCAST_ADDR[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

#define IE_SSID                         0
#define IE_SUPP_RATES                   1
#define IE_FH_PARM                      2
#define IE_DS_PARM                      3
#define IE_CF_PARM                      4
const UCHAR  SsidIe	 = IE_SSID;
const UCHAR  SupRateIe = IE_SUPP_RATES;
const UCHAR  DsIe 	 = IE_DS_PARM;
/**
 * @brief  ����wifiģʽΪAP������AP������δ���ƣ�
 *
 * @param  name : ��������
 * @param  key:����
 * @param  key_type:����ģʽ,��ѡ����KEY_NONE, KEY_WEP,KEY_WPA,KEY_WPA2
 * @param  channel:Ƶ��
 * @param  max_client:�������ͻ�������,1 - MAX_STA_COUNT(SDKԤ��Ϊ10)
 *
 * @retval ok: 0, err: -1
 */
int wifi_ap_cfg(char *name, char *key, int key_type, int channel, int max_client)
{
	struct cfg80211_ap_settings set;
	struct rtw_ieee80211_hdr_3addr hdr;
	struct	mlme_priv	*pmlmepriv;
	_adapter *padapter;
	uint8_t *pBeaconFrame;
	uint32_t FakeTimestamp;
	uint16_t BeaconPeriod, CapabilityInfo, SsidLen;
	uint8_t DsLen = 1;
	unsigned char			bssrate[NumRates];
	int	bssrate_len = 0;
	int FrameLen;
	int ret = -1;
	p_dbg_enter;

	if(!wifi_dev)
		return -1;

	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);
	
	pmlmepriv = &(padapter->mlmepriv);
	
	pBeaconFrame = (uint8_t*)mem_calloc(512, 1);
	if(!pBeaconFrame)
		return -1;

	FakeTimestamp = os_time_get();
	BeaconPeriod = 100;
	CapabilityInfo = WLAN_CAPABILITY_BSS | WLAN_CAPABILITY_SHORT_PREAMBLE | WLAN_CAPABILITY_SHORT_SLOT/* |WLAN_CAPABILITY_PRIVACY*/;
	SsidLen = strlen(name);
	memset(&set, 0, sizeof(struct cfg80211_ap_settings));

	get_rate_set(padapter, bssrate, &bssrate_len);

	MgtMacHeaderInit((struct HEADER_802_11*)&hdr, SUBTYPE_BEACON, 0, (PUCHAR)BROADCAST_ADDR, 
						wifi_dev->dev_addr);

	MakeOutgoingFrame(pBeaconFrame,                  (u32*)&FrameLen,
					24,           &hdr, 
					TIMESTAMP_LEN,                   &FakeTimestamp,
					2,                               &BeaconPeriod,
					2,                               &CapabilityInfo,
					1,                               &SsidIe, 
					1,                               &SsidLen, 
					SsidLen,                      name,
					1,                               &SupRateIe, 
					1,                               &bssrate_len,
					bssrate_len,                bssrate, 
					1,                               &DsIe, 
					1,                               &DsLen, 
					1,                               &channel,
					-1);

	set.beacon.head = pBeaconFrame;
	set.beacon.head_len = FrameLen;

	set.ssid = (const u8*)name;
	set.ssid_len = SsidLen;
	
	set_fwstate(pmlmepriv, WIFI_AP_STATE);
	
	if(wifi_dev->cfg80211_ops->start_ap)
		ret = wifi_dev->cfg80211_ops->start_ap(padapter->rtw_wdev->wiphy, 
			wifi_dev,
			&set);

	mem_free(pBeaconFrame);
	return ret;
}


/**
 * @brief  wifi����,�����Զ��������ģʽ��Ƶ��,�û�ֻ���ṩ·�������ƺ�����
 * @param
 *							
 *		essid: ��AP����
 *		key: ��������	
 * @retval ok: 0, err: -1
 */
int wifi_connect( char *essid, char *key)
{
	int result_ret = 0;
	_adapter *padapter;
	int essid_len = strlen(essid);
	int key_len = strlen(key);
	struct cfg80211_connect_params *smee = 0;
	u8_t tmp[16];
	
	p_dbg("enter %s\n", __FUNCTION__);
	if(essid_len > 32 || key_len > 32)
	{
		result_ret = -1;
		goto ret;
	}

	if(!wifi_dev)
		return -1;

	//smee ��wifi�����ڲ��ͷ�
	smee = (struct cfg80211_connect_params *)mem_malloc(sizeof(struct cfg80211_connect_params));
	if(smee == 0)
	{
		result_ret = -1;
		goto ret;
	}
	
	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);

	//priv = (moal_private *)wifi_dev->ml_priv;
	memset(smee,0,sizeof(struct cfg80211_connect_params));
	smee->bssid = 0; 
	
	memcpy(smee->ssid,essid,essid_len);
	smee->ssid[essid_len] = 0;
	
	smee->ssid_len = essid_len;
	
	memcpy(smee->key,key,key_len);
	smee->key[key_len] = 0;
	
	smee->auth_type = NL80211_AUTHTYPE_AUTOMATIC;
	smee->crypto.wpa_versions = NL80211_WPA_VERSION_2;
	smee->crypto.cipher_group = WLAN_CIPHER_SUITE_CCMP;//WLAN_CIPHER_SUITE_CCMP;
	smee->crypto.n_ciphers_pairwise = 1;
	smee->crypto.ciphers_pairwise[0] = WLAN_CIPHER_SUITE_CCMP;//WLAN_CIPHER_SUITE_CCMP;
	smee->privacy = 0;

	memset(smee,0,sizeof(struct cfg80211_connect_params));
	smee->bssid = /*arg->bssid*/0;  //mark
	
	memcpy(smee->ssid,essid,essid_len);
	smee->ssid[essid_len] = 0;
	
	smee->ssid_len = essid_len;
	
	memcpy(smee->key,key,key_len);
	smee->key[key_len] = 0;
	
	smee->key_len = key_len;
	smee->crypto.cipher_group = /*WLAN_CIPHER_SUITE_CCMP*/0;

	//����PMK
	pbkdf2_sha1((const char*)smee->key, (const char*)smee->ssid, smee->ssid_len, 8, tmp, PMKID_LEN);
	if(memcmp(tmp,pmksa[0].id,PMKID_LEN))
	{
		memcpy(pmksa[0].id,tmp,PMKID_LEN);
		pbkdf2_sha1((const char*)smee->key, (const char*)smee->ssid, smee->ssid_len, 4096, pmksa[0].pmk, 32);
	}
	pmksa[0].pmk_len = PMK_LEN;

	padapter->sme = (void*)smee;
	
	/*
	smee->auth_type = NL80211_AUTHTYPE_SHARED_KEY;
	smee->crypto.wpa_versions = 0;
	smee->crypto.cipher_group = WLAN_CIPHER_SUITE_WEP40;//WLAN_CIPHER_SUITE_CCMP;
	smee->crypto.n_ciphers_pairwise = 1;
	smee->crypto.ciphers_pairwise[0] = WLAN_CIPHER_SUITE_WEP40;//WLAN_CIPHER_SUITE_CCMP;
	smee->privacy = 1;
	*/
	result_ret = wifi_dev->cfg80211_ops->wifi_connect(padapter->rtw_wdev->wiphy, wifi_dev,smee);
ret:
	if(result_ret)
		p_err("user_link_app err:%d\n",result_ret);

	p_dbg("exit %s\n", __FUNCTION__);
	
	return result_ret;
}

/**
 * @brief  �Ͽ�����
 */
int wifi_disconnect()
{
	int ret = -1;
	_adapter *padapter;

	if(!wifi_dev)
		return -1;
	
	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);
	
	if(wifi_dev->cfg80211_ops && wifi_dev->cfg80211_ops->disconnect)
	{
		ret = wifi_dev->cfg80211_ops->disconnect(padapter->rtw_wdev->wiphy, wifi_dev, 0);

	}else
		p_err("%s not support", __FUNCTION__);
	
	return ret;
}


/**
 * @brief  ��ȡmac��ַ��mac��ַ��8782оƬ��ȡ
 * @param  mac_addr : mac��ַ 6BYTE
 */
int wifi_get_mac(unsigned char *mac_addr)
{
	if(!wifi_dev || !mac_addr)
		return -1;
	
	memcpy(mac_addr, wifi_dev->dev_addr, ETH_ALEN);
	return 0;
}


/**
 * @brief  ��ȡWIFI���ӵ�ͳ����Ϣ(�ź�ǿ��...)
 * @param  pStats : ָ��station_info��ָ��
 * @param  mac : ָ����Ҫ��ȡ��station�ĵ�ַ
 */
int wifi_get_stats(uint8_t *mac,struct station_info *sinfo)
{

	int ret = -1;
	_adapter *padapter;

	if(!wifi_dev)
		return -1;
	
	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);
	
	if(wifi_dev->cfg80211_ops && wifi_dev->cfg80211_ops->get_station)
	{
		ret = wifi_dev->cfg80211_ops->get_station(padapter->rtw_wdev->wiphy, wifi_dev, mac,sinfo);

	}else
		p_err("%s not support", __FUNCTION__);
	
	return ret;
}

/**
 * @brief  ����ʡ��ģʽ,�ڱ������ӵ�����¿ɴﵽ�ܿɹ�ʡ��Ч��
 * @param  power_save_level : 
 */
int wifi_power_cfg(uint8_t power_save_level)
{
	_adapter *padapter;

	if(!wifi_dev)
		return -1;
	
	padapter = (_adapter *)rtw_netdev_priv(wifi_dev);

	if(power_save_level)
		LPS_Enter(padapter);
	else
		LPS_Leave(padapter);

	return 0;
}




extern uint8_t g_mac_addr[6];

/**
 * @brief  ͨ��CPU_ID�����һ��������������MAC��ַ�ĵ�3�ֽ�
 *
 *
 */
int create_mac(unsigned char *mac)
{
	char psk[33], id[12];

	int i, j, k, ret = 0;
	i =  *CPU_ID;
	j = *(CPU_ID + 1);
	k = *(CPU_ID + 2);
	memcpy(id, &i, 4);
	memcpy(id + 4, &j, 4);
	memcpy(id + 8, &k, 4);
	//ret = pbkdf2_sha1(ps, id, 12, 100, (u8*)psk, 32);
	//ȡcpu id��������mac��ַ
	psk[0] = i;
	psk[1] = j;
	psk[2] = k;

	//ȡ�����ֽ���Ϊmac��ַ
	mac[0] = 0x00;
	mac[1] = 0x0c;
	mac[2] = 0x43;
	memcpy(mac + 3, psk, 3);
	mac[5] = mac[5] &0xfe; //for 2-BSSID mode

	dump_hex("mac", mac, 6);

	return ret;
}

extern int rtw_drv_init(struct sdio_func *func);
int init_wifi()
{
	int ret;

	func.vendor = 0x024c;
	func.device = 0x8179;

	ret = rtw_drv_init(&func);
	if(!ret && wifi_dev && wifi_dev->netdev_ops && wifi_dev->netdev_ops->ndo_open)
	{
        	ret = wifi_dev->netdev_ops->ndo_open(wifi_dev);
       		if(!ret)
			init_wpa_supplicant();
	}else{
        	p_err_fun;
	}
	return ret;
}



