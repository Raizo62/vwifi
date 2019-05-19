#ifndef _CSOCKETSERVER_
#define _CSOCKETSERVER_

#include "cvsocket.h"

const int MAX_CLIENT = 30;

class CVSocketServer : public CVSocket
{
	int Port;
	int SocketClients[MAX_CLIENT];
	int NumberClient;

	public :

	void Init(int port);

	bool Listen();

	int Accept();

	int GetSocketClient(int number);

	int GetNumberClient();

	void CloseClient(int number);

	void SendAllOtherClients(int number,const char* data, ssize_t sizeOfData);

};

#endif
