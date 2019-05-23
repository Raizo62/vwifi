#ifndef _CSOCKETSERVER_
#define _CSOCKETSERVER_

#include "cvsocket.h"

const int MAX_CLIENT = 30;

class CVSocketServer : public CVSocket
{
		unsigned int Port;
		Descriptor SocketClients[MAX_CLIENT];
		unsigned int NumberClient;

		Descriptor GetSocketClient(unsigned int number);

	public :

		CVSocketServer();

		CVSocketServer(TypeSocket type);

		~CVSocketServer();

		void Init(int port);

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
