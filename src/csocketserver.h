#ifndef _CSOCKETSERVER_
#define _CSOCKETSERVER_

#include "config.h"
#include "csocket.h"

class CSocketServer : public CSocket
{
	protected :

		unsigned int Port;

		unsigned int NumberClient;
		Descriptor SocketClients[MAX_CLIENT];

		Descriptor Accept(struct sockaddr_in& address);

		Descriptor GetSocketClient(unsigned int number);

	public :

		CSocketServer();

		CSocketServer(TypeSocket type);

		~CSocketServer();

		void Init(unsigned int port);

		bool Listen();

		virtual Descriptor Accept();

		unsigned int GetNumberClient();

		void CloseClient(unsigned int number);

		ssize_t Send(Descriptor descriptor, const char* data, ssize_t sizeOfData);

		ssize_t Read(Descriptor descriptor, char* data, ssize_t sizeOfData);

		Descriptor operator[] (unsigned int number)
		{
			return GetSocketClient(number);
		}
};

#endif
