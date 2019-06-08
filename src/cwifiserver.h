#ifndef _CWIFISERVER_H_
#define _CWIFISERVER_H_

#include "csocketserver.h"
#include "cinfowifi.h"

class CWifiServer : public CSocketServer
{
		CInfoWifi* InfoClient;

	public :

		bool Listen(Number maxClient);

		Descriptor Accept();

		void ShowInfoClient(Number number);

		void CloseClient(Number number);

		void SendAllOtherClients(Number number,const char* data, ssize_t sizeOfData);

		CInfoWifi* GetReferenceOnInfoClientByCID(TypeCID cid);

		CInfoWifi* GetReferenceOnInfoClientByNumber(Number number);

};

#endif
