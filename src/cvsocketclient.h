#ifndef _CSOCKETSERVER_
#define _CSOCKETSERVER_

#include "cvsocket.h"

class CVSocketClient : public CVSocket
{
	bool IsConnected;

	public :

		CVSocketClient();

		CVSocketClient(TypeSocket type);

		void Init();

		// TypeSocket : AF_INET :
		bool Connect(const char* IP, unsigned int port);

		// TypeSocket : AF_VSOCK :
		bool Connect(unsigned int port);

		ssize_t Send(const char* data, ssize_t sizeOfData);

		ssize_t Read(char* data, ssize_t sizeOfData);
};

#endif
