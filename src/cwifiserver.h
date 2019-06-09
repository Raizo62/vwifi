#ifndef _CWIFISERVER_H_
#define _CWIFISERVER_H_

#include "csocketserver.h"
#include "cinfowifi.h"

class CWifiServer : public CSocketServer
{
		CInfoWifi* InfoClient;

	public :

		bool Listen(TNumber maxClient);

		TDescriptor Accept();

		void ShowInfoClient(TNumber number);

		void CloseClient(TNumber number);

		void SendAllOtherClients(TNumber number,const char* data, ssize_t sizeOfData);

		CInfoWifi* GetReferenceOnInfoClientByCID(TCID cid);

		CInfoWifi* GetReferenceOnInfoClientByNumber(TNumber number);

};

#endif
