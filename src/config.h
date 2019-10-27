#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "types.h" // TIndex / TPort

const TIndex WIFI_MAX_DECONNECTED_CLIENT = 15;

const TPort WIFI_GUEST_PORT = 8211;
const TPort WIFI_HOST_PORT = 8211;

#ifndef _DEBUG
	#define _USE_VSOCK_BY_DEFAULT_
#endif

const TPort CTRL_PORT = WIFI_GUEST_PORT+1;

#ifndef _DEBUG
	#define ADDRESS_IP "127.0.0.1"
#else
	#define ADDRESS_IP "192.168.56.101"
#endif

#endif
