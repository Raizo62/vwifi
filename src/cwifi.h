#ifndef _WIFI_H_
#define _WIFI_H_

#include "types.h" // TPower
#include "csocket.h" // CSocket

class CWifi
{
	protected :

		TFrequency GetFrequency(struct nlmsghdr* nlh);

		// distance : meter
		int Attenuation(TDistance distance, TFrequency frequency);

		// return power value between [TPower_MIN,TPower_MAX]
		TPower BoundedPower(int power);

		bool PacketIsLost(TPower signalLevel);

		ssize_t SendSignalWithSocket(CSocket* socket, TDescriptor descriptor, TPower* power, uint8_t *dropped, const char* buffer, int sizeOfBuffer);
		ssize_t RecvSignalWithSocket(CSocket* socket, TDescriptor descriptor, TPower* power, uint8_t *dropped, CDynBuffer* buffer);
};

#endif
