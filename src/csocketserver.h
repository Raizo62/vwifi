#ifndef _CSOCKETSERVER_H_
#define _CSOCKETSERVER_H_

#include "csocket.h"
#include "types.h" // TDescriptor

class CSocketServer : public CSocket
{
	protected :

		TPort Port;

		Number MaxClient;
		Number NumberClient;

		TDescriptor* SocketClients;

		TDescriptor Accept(struct sockaddr_in& address);

		TDescriptor GetSocketClient(Number number);

	public :

		CSocketServer();

		CSocketServer(TypeSocket type);

		~CSocketServer();

		void Init(TPort port);

		virtual bool Listen(Number maxClient);

		virtual TDescriptor Accept();

		Number GetNumberClient();

		void CloseClient(Number number);

		ssize_t Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData);

		ssize_t Read(TDescriptor descriptor, char* data, ssize_t sizeOfData);

		TDescriptor operator[] (Number number)
		{
			return GetSocketClient(number);
		}
};

#endif
