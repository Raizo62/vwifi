#ifndef _CWIFISERVER_
#define _CWIFISERVER_

#include "csocketserver.h"
#include "cinfowifi.h"

class CWifiServer : public CSocketServer
{
		CInfoWifi InfoClient[MAX_CLIENT];

	public :

		Descriptor Accept();

		void ShowInfoClient(unsigned int number);

		void CloseClient(unsigned int number);

		void SendAllOtherClients(unsigned int number,const char* data, ssize_t sizeOfData);

		CInfoWifi*	GetReferenceOnInfoClient(TypeCID cid);

};

#endif
