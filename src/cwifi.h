#ifndef _WIFI_H_
#define _WIFI_H_

#include "types.h" // TPower
#include "csocket.h" // CSocket

class CWifi
{
	protected :

		// distance : meter
		int Attenuation(TDistance distance);

		// return power value between [TPower_MIN,TPower_MAX]
		TPower BoundedPower(int power);

		bool PacketIsLost(TPower signalLevel);

		ssize_t SendSignalWithSocket(CSocket* socket, TDescriptor descriptor, TPower* power, const char* buffer, int sizeOfBuffer);
		ssize_t RecvSignalWithSocket(CSocket* socket, TDescriptor descriptor, TPower* power, CDynBuffer* buffer);
};

#endif
