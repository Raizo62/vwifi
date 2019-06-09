#ifndef _CSOCKETSERVER_H_
#define _CSOCKETSERVER_H_

#include "csocket.h"
#include "types.h" // TDescriptor

class CSocketServer : public CSocket
{
	protected :

		TPort Port;

		TNumber MaxClient;
		TNumber NumberClient;

		TDescriptor* SocketClients;

		TDescriptor Accept(struct sockaddr_in& address);

		TDescriptor GetSocketClient(TNumber number);

	public :

		CSocketServer();

		CSocketServer(TypeSocket type);

		~CSocketServer();

		void Init(TPort port);

		virtual bool Listen(TNumber maxClient);

		virtual TDescriptor Accept();

		TNumber GetNumberClient();

		void CloseClient(TNumber number);

		ssize_t Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData);

		ssize_t Read(TDescriptor descriptor, char* data, ssize_t sizeOfData);

		TDescriptor operator[] (TNumber number)
		{
			return GetSocketClient(number);
		}
};

#endif
