#ifndef _CWIFISERVER_
#define _CWIFISERVER_

#include "csocketserver.h"

class CWifiServer : public CSocketServer
{
		int InfoClient[MAX_CLIENT];

	public :

		Descriptor Accept();

		void ShowInfoClient(unsigned int number);

		void CloseClient(unsigned int number);

		void SendAllOtherClients(unsigned int number,const char* data, ssize_t sizeOfData);

};

#endif
