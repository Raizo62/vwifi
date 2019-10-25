#ifndef _CSOCKETCLIENT_H_
#define _CSOCKETCLIENT_H_

#include <netinet/ip.h> // struct sockaddr_in
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "csocket.h"

class CSocketClient : public CSocket
{
		union TypeSocketClient
		{
			struct sockaddr_in inet;
			struct sockaddr_vm vhost;
		};

		bool UseSocketVHOST;
		TypeSocketClient Server;

		bool IsConnected;

		bool StopTheReconnect;

		void Init();

		bool ConnectLoop(struct sockaddr* server, size_t size_of_server);

	public :

		CSocketClient();
		~CSocketClient();

		CSocketClient(TSocket type);

		// TSocket : AF_INET :
		void Init(const char* IP, TPort port);

		// TSocket : AF_VSOCK :
		void Init(TPort port);

		bool Connect();

		ssize_t Send(const char* data, ssize_t sizeOfData);

		ssize_t Read(char* data, ssize_t sizeOfData);
		
		bool SetBlocking(bool blocking);

		void StopReconnect(bool status);
};

#endif
