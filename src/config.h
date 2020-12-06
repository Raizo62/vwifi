#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "types.h" // TIndex / TPort

const TIndex WIFI_MAX_DECONNECTED_CLIENT = 15;

const bool LOST_PACKET_BY_DEFAULT=false;

#define _USE_VSOCK_BY_DEFAULT_

const TPort WIFI_GUEST_PORT_VHOST = 8211;
const TPort WIFI_GUEST_PORT_INET = WIFI_GUEST_PORT_VHOST+1;

const TPort WIFI_SPY_PORT = WIFI_GUEST_PORT_VHOST+2;

const TPort CTRL_PORT = WIFI_GUEST_PORT_VHOST+3;

#define ADDRESS_IP "127.0.0.1"

#ifdef _DEBUG
	// #define _VERBOSE1
	// #define _VERBOSE2
#endif

#endif
