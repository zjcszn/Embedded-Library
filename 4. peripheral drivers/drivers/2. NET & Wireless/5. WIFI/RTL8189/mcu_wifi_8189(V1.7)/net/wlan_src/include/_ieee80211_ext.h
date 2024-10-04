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
#ifndef __IEEE80211_EXT_H
#define __IEEE80211_EXT_H

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

#define WMM_OUI_TYPE 2
#define WMM_OUI_SUBTYPE_INFORMATION_ELEMENT 0
#define WMM_OUI_SUBTYPE_PARAMETER_ELEMENT 1
#define WMM_OUI_SUBTYPE_TSPEC_ELEMENT 2
#define WMM_VERSION 1

#define WPA_PROTO_WPA BIT(0)
#define WPA_PROTO_RSN BIT(1)

#define WPA_KEY_MGMT_IEEE8021X BIT(0)
#define WPA_KEY_MGMT_PSK BIT(1)
#define WPA_KEY_MGMT_NONE BIT(2)
#define WPA_KEY_MGMT_IEEE8021X_NO_WPA BIT(3)
#define WPA_KEY_MGMT_WPA_NONE BIT(4)


#define WPA_CAPABILITY_PREAUTH BIT(0)
#define WPA_CAPABILITY_MGMT_FRAME_PROTECTION BIT(6)
#define WPA_CAPABILITY_PEERKEY_ENABLED BIT(9)


#define PMKID_LEN 16


#ifdef PLATFORM_LINUX
struct wpa_ie_hdr {
	u8 elem_id;
	u8 len;
	u8 oui[4]; /* 24-bit OUI followed by 8-bit OUI type */
	u8 version[2]; /* little endian */
}__attribute__ ((packed));

struct rsn_ie_hdr {
	u8 elem_id; /* WLAN_EID_RSN */
	u8 len;
	u8 version[2]; /* little endian */
}__attribute__ ((packed));

struct wme_ac_parameter {
#if defined(CONFIG_LITTLE_ENDIAN)
	/* byte 1 */
	u8 	aifsn:4,
		acm:1,
	 	aci:2,
	 	reserved:1;

	/* byte 2 */
	u8 	eCWmin:4,
	 	eCWmax:4;
#elif defined(CONFIG_BIG_ENDIAN)
	/* byte 1 */
	u8 	reserved:1,
	 	aci:2,
	 	acm:1,
	 	aifsn:4;

	/* byte 2 */
	u8 	eCWmax:4,
	 	eCWmin:4;
#else
#error	"Please fix <endian.h>"
#endif

	/* bytes 3 & 4 */
	u16 txopLimit;
} __attribute__ ((packed));

struct wme_parameter_element {
	/* required fields for WME version 1 */
	u8 oui[3];
	u8 oui_type;
	u8 oui_subtype;
	u8 version;
	u8 acInfo;
	u8 reserved;
	struct wme_ac_parameter ac[4];

} __attribute__ ((packed));

#endif

#ifdef PLATFORM_WINDOWS

#pragma pack(1)

struct wpa_ie_hdr {
	u8 elem_id;
	u8 len;
	u8 oui[4]; /* 24-bit OUI followed by 8-bit OUI type */
	u8 version[2]; /* little endian */
};

struct rsn_ie_hdr {
	u8 elem_id; /* WLAN_EID_RSN */
	u8 len;
	u8 version[2]; /* little endian */
};

#pragma pack()

#endif

#define WPA_PUT_LE16(a, val)			\
	do {					\
		(a)[1] = ((u16) (val)) >> 8;	\
		(a)[0] = ((u16) (val)) & 0xff;	\
	} while (0)

#define WPA_PUT_BE32(a, val)					\
	do {							\
		(a)[0] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[3] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define WPA_PUT_LE32(a, val)					\
	do {							\
		(a)[3] = (u8) ((((u32) (val)) >> 24) & 0xff);	\
		(a)[2] = (u8) ((((u32) (val)) >> 16) & 0xff);	\
		(a)[1] = (u8) ((((u32) (val)) >> 8) & 0xff);	\
		(a)[0] = (u8) (((u32) (val)) & 0xff);		\
	} while (0)

#define RSN_SELECTOR_PUT(a, val) WPA_PUT_BE32((u8 *) (a), (val))
//#define RSN_SELECTOR_PUT(a, val) WPA_PUT_LE32((u8 *) (a), (val))

/* mgmt header + 1 byte category code */
//#define IEEE80211_MIN_ACTION_SIZE FIELD_OFFSET(struct ieee80211_mgmt, u.action.u)



#endif

