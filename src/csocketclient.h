#ifndef _CSOCKETCLIENT_
#define _CSOCKETCLIENT_

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
		bool Connect(const char* IP, unsigned int port);

		// TypeSocket : AF_VSOCK :
		bool Connect(unsigned int port);

		ssize_t Send(const char* data, ssize_t sizeOfData);

		ssize_t Read(char* data, ssize_t sizeOfData);
		
		int SetBloking(int blocking); 
};

#endif
