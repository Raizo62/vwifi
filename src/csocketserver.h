#ifndef _CSOCKETSERVER_
#define _CSOCKETSERVER_

#include "csocket.h"

const int MAX_CLIENT = 30;

class CSocketServer : public CSocket
{
		unsigned int Port;
		Descriptor SocketClients[MAX_CLIENT];
		unsigned int NumberClient;

		Descriptor GetSocketClient(unsigned int number);

	public :

		CSocketServer();

		CSocketServer(TypeSocket type);

		~CSocketServer();

		void Init(unsigned int port);

		bool Listen();

		Descriptor Accept();

		unsigned int GetNumberClient();

		void CloseClient(unsigned int number);

		void SendAllOtherClients(unsigned int number,const char* data, ssize_t sizeOfData);

		ssize_t Send(Descriptor descriptor, const char* data, ssize_t sizeOfData);

		ssize_t Read(Descriptor descriptor, char* data, ssize_t sizeOfData);

		Descriptor operator[] (unsigned int number)
		{
			return GetSocketClient(number);
		}
};

#endif
