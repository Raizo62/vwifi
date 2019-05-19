#ifndef _CSOCKETSERVER_
#define _CSOCKETSERVER_

#include "cvsocket.h"

class CVSocketClient : public CVSocket
{
	bool IsConnected;

	public :

	CVSocketClient();

	void Init();

	bool Connect(const char* IP, int port);

	ssize_t Send(const char* data, ssize_t sizeOfData);

	ssize_t Read(char* data, ssize_t sizeOfData);
};

#endif
