#ifndef _CWIFISERVER_H_
#define _CWIFISERVER_H_

#include <vector> // vector

#include "csocketserver.h"
#include "cinfowifi.h"

class CWifiServer : public CSocketServer
{
		TIndex MaxClientDeconnected;

		std::vector<CInfoWifi> InfoClients;
		std::vector<CInfoWifi> InfoClientsDeconnected;

		bool RecoverCoordinateOfOldInfoClient(TCID cid, CCoordinate& coo);

		bool RecoverCoordinateOfInfoClient(TCID cid, CCoordinate& coo);

	public :

		bool Listen(TIndex maxClientDeconnected);

		TDescriptor Accept();

		bool IsEnable(TIndex index);

		void ShowInfoClient(TIndex index);

		void CloseClient(TIndex index);

		void SendAllOtherClients(TIndex index,const char* data, ssize_t sizeOfData);

		CInfoWifi* GetReferenceOnInfoClientByCID(TCID cid);

		CInfoWifi* GetReferenceOnInfoClientDeconnectedByCID(TCID cid);

		CInfoWifi* GetReferenceOnInfoClientByIndex(TIndex index);

};

#endif
