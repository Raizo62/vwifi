#ifndef _CSOCKETSERVER_
#define _CSOCKETSERVER_

#include "csocket.h"

const int MAX_CLIENT = 30;

class CSocketServer : public CSocket
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

};

#endif
