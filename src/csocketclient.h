#ifndef _CSOCKETCLIENT_H_
#define _CSOCKETCLIENT_H_

#include "csocket.h"

class CSocketClient : public CSocket
{
		bool IsConnected;

		bool ConnectLoop(struct sockaddr* server, size_t size_of_server);

	public :

		CSocketClient();
		~CSocketClient();

		CSocketClient(TSocket type);

		void Init();

		// TSocket : AF_INET :
		bool Connect(const char* IP, TPort port);

		// TSocket : AF_VSOCK :
		bool Connect(TPort port);

		ssize_t Send(const char* data, ssize_t sizeOfData);

		ssize_t Read(char* data, ssize_t sizeOfData);
		
		bool SetBlocking(bool blocking);
};

#endif
