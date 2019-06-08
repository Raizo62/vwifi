#ifndef _CSOCKETCLIENT_H_
#define _CSOCKETCLIENT_H_

#include "csocket.h"

class CSocketClient : public CSocket
{
	bool IsConnected;

	public :

		CSocketClient();
		~CSocketClient();

		CSocketClient(TypeSocket type);

		void Init();

		// TypeSocket : AF_INET :
		bool Connect(const char* IP, TPort port);

		// TypeSocket : AF_VSOCK :
		bool Connect(TPort port);

		ssize_t Send(const char* data, ssize_t sizeOfData);

		ssize_t Read(char* data, ssize_t sizeOfData);
};

#endif
