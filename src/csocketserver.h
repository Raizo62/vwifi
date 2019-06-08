#ifndef _CSOCKETSERVER_H_
#define _CSOCKETSERVER_H_

#include "csocket.h"

typedef unsigned int Number;

class CSocketServer : public CSocket
{
	protected :

		TPort Port;

		Number MaxClient;
		Number NumberClient;

		Descriptor* SocketClients;

		Descriptor Accept(struct sockaddr_in& address);

		Descriptor GetSocketClient(Number number);

	public :

		CSocketServer();

		CSocketServer(TypeSocket type);

		~CSocketServer();

		void Init(TPort port);

		virtual bool Listen(Number maxClient);

		virtual Descriptor Accept();

		Number GetNumberClient();

		void CloseClient(Number number);

		ssize_t Send(Descriptor descriptor, const char* data, ssize_t sizeOfData);

		ssize_t Read(Descriptor descriptor, char* data, ssize_t sizeOfData);

		Descriptor operator[] (Number number)
		{
			return GetSocketClient(number);
		}
};

#endif
