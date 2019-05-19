#ifndef _CSOCKETSERVER_
#define _CSOCKETSERVER_

#include "cvsocket.h"

class CVSocketClient : public CVSocket
{
	bool IsConnected;

	public :

	void Init(int port);

	bool Connect(char* IP, int port);
};

#endif
