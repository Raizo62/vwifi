#ifndef _CSOCKETCLIENT_H_
#define _CSOCKETCLIENT_H_

#include <netinet/ip.h> // struct sockaddr_in
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "csocket.h"

class CSocketClient : public CSocket
{
		bool IsConnected;

		bool StopTheReconnect;

		void Init();

	protected :

		bool ConnectCore(struct sockaddr* server, size_t size_of_server);

	public :

		CSocketClient();
		~CSocketClient();

		CSocketClient(TSocket type);

		ssize_t Send(const char* data, ssize_t sizeOfData);
		ssize_t SendBigData(const char* data, TMinimalSize sizeOfData);

		ssize_t Read(char* data, ssize_t sizeOfData);
		ssize_t ReadBigData(CDynBuffer* data);

		bool ConnectLoop();

	// virtual :
		virtual bool Connect()=0;


		virtual int GetID()=0;
};

#endif
