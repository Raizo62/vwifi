#ifndef IEEE80211_H_
#define IEEE80211_H_

#include <stdint.h> // added by boussad


#define u8 uint8_t

#define IEEE80211_AVAILABLE_RATES 12
#define IEEE80211_MAX_RATES_PER_TX 5

#define IEEE80211_FTYPE_MGMT            0x0000
#define IEEE80211_FTYPE_CTL             0x0004
#define IEEE80211_FTYPE_DATA            0x0008
#define IEEE80211_FTYPE_EXT             0x000c

/* control */
#define IEEE80211_STYPE_CTL_EXT         0x0060
#define IEEE80211_STYPE_BACK_REQ        0x0080
#define IEEE80211_STYPE_BACK            0x0090
#define IEEE80211_STYPE_PSPOLL          0x00A0
#define IEEE80211_STYPE_RTS             0x00B0
#define IEEE80211_STYPE_CTS             0x00C0
#define IEEE80211_STYPE_ACK             0x00D0
#define IEEE80211_STYPE_CFEND           0x00E0
#define IEEE80211_STYPE_CFENDACK        0x00F0

/**
 *	\brief IEEE80211 header
 *
 *	IEEE80211 header fields
 */
/*
struct ieee80211_hdr {
	unsigned char frame_control[2];
	unsigned char duration_id[2];
	unsigned char addr1[6];
	unsigned char addr2[6];
	unsigned char addr3[6];
	unsigned char seq_ctrl[2];
	unsigned char addr4[6];
};
*/

struct ieee80211_hdr {
        __le16 frame_control;
        __le16 duration_id;
        u8 addr1[ETH_ALEN];
        u8 addr2[ETH_ALEN];
        u8 addr3[ETH_ALEN];
        __le16 seq_ctrl;
        u8 addr4[ETH_ALEN];
};

#endif /* IEEE80211_H_ */

