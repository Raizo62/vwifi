#ifndef _CWIFISERVER_H_
#define _CWIFISERVER_H_

#include "csocketserver.h"
#include "cinfowifi.h"

class CWifiServer : public CSocketServer
{
		CInfoWifi* InfoClient;

	public :

		bool Listen(TIndex maxClient);

		TDescriptor Accept();

		void ShowInfoClient(TIndex index);

		void CloseClient(TIndex index);

		void SendAllOtherClients(TIndex index,const char* data, ssize_t sizeOfData);

		CInfoWifi* GetReferenceOnInfoClientByCID(TCID cid);

		CInfoWifi* GetReferenceOnInfoClientByIndex(TIndex index);

};

#endif
