/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#define _RTL8188E_SRESET_C_

#include <rtl8188e_sreset.h>
#include <rtl8188e_hal.h>

#ifdef DBG_CONFIG_ERROR_DETECT

void rtl8188e_sreset_xmit_status_check(_adapter *padapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	//unsigned long current_time;
	//struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	//unsigned int diff_time;
	u32 txdma_status;
	txdma_status=txdma_status;
	if( (txdma_status=rtw_read32(padapter, REG_TXDMA_STATUS)) !=0x00){
		DBG_871X("%s REG_TXDMA_STATUS:0x%08x\n", __FUNCTION__, txdma_status);	
		rtw_hal_sreset_reset(padapter);
	}
#ifdef CONFIG_USB_HCI
	//total xmit irp = 4
	//DBG_8192C("==>%s free_xmitbuf_cnt(%d),txirp_cnt(%d)\n",__FUNCTION__,pxmitpriv->free_xmitbuf_cnt,pxmitpriv->txirp_cnt);
	//if(pxmitpriv->txirp_cnt == NR_XMITBUFF+1)
	current_time = rtw_get_current_time();

	if(0 == pxmitpriv->free_xmitbuf_cnt || 0 == pxmitpriv->free_xmit_extbuf_cnt) {

		diff_time = rtw_get_passing_time_ms(psrtpriv->last_tx_time);

		if (diff_time > 2000) {
			if (psrtpriv->last_tx_complete_time == 0) {
				psrtpriv->last_tx_complete_time = current_time;
			}
			else{
				diff_time = rtw_get_passing_time_ms(psrtpriv->last_tx_complete_time);
				if (diff_time > 4000) {
					//padapter->Wifi_Error_Status = WIFI_TX_HANG;
					DBG_871X("%s tx hang\n", __FUNCTION__);
					rtw_hal_sreset_reset(padapter);
				}
			}
		}
	}
#endif //CONFIG_USB_HCI

	if (psrtpriv->dbg_trigger_point == SRESET_TGP_XMIT_STATUS) {
		psrtpriv->dbg_trigger_point = SRESET_TGP_NULL;
		rtw_hal_sreset_reset(padapter);
		return;
	}
}

void rtl8188e_sreset_linked_status_check(_adapter *padapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	u32 rx_dma_status = 0;
	u8 fw_status=0;
	rx_dma_status = rtw_read32(padapter,REG_RXDMA_STATUS);
	if(rx_dma_status!= 0x00){
		DBG_8192C("%s REG_RXDMA_STATUS:0x%08x \n",__FUNCTION__,rx_dma_status);
		rtw_write32(padapter,REG_RXDMA_STATUS,rx_dma_status);
	}	
	fw_status = rtw_read8(padapter,REG_FMETHR);
	if(fw_status != 0x00)
	{		
		if(fw_status == 1)
			DBG_8192C("%s REG_FW_STATUS (0x%02x), Read_Efuse_Fail !!   \n",__FUNCTION__,fw_status);
		else if(fw_status == 2)
			DBG_8192C("%s REG_FW_STATUS (0x%02x), Condition_No_Match !!   \n",__FUNCTION__,fw_status);
	}
#if 0
	u32 regc50,regc58,reg824,reg800;
	regc50 = rtw_read32(padapter,0xc50);
	regc58 = rtw_read32(padapter,0xc58);
	reg824 = rtw_read32(padapter,0x824);
	reg800 = rtw_read32(padapter,0x800);
	if(	((regc50&0xFFFFFF00)!= 0x69543400)||
		((regc58&0xFFFFFF00)!= 0x69543400)||
		(((reg824&0xFFFFFF00)!= 0x00390000)&&(((reg824&0xFFFFFF00)!= 0x80390000)))||
		( ((reg800&0xFFFFFF00)!= 0x03040000)&&((reg800&0xFFFFFF00)!= 0x83040000)))
	{
		DBG_8192C("%s regc50:0x%08x, regc58:0x%08x, reg824:0x%08x, reg800:0x%08x,\n", __FUNCTION__,
			regc50, regc58, reg824, reg800);
		rtw_hal_sreset_reset(padapter);
	}
#endif

	if (psrtpriv->dbg_trigger_point == SRESET_TGP_LINK_STATUS) {
		psrtpriv->dbg_trigger_point = SRESET_TGP_NULL;
		rtw_hal_sreset_reset(padapter);
		return;
	}
}
#endif

