#ifndef _CCTRLSERVER_H_
#define _CCTRLSERVER_H_

#include "cwifiserver.h" // CWifiServer CSocketServer
#include "types.h" // TOrder

class CCTRLServer : public CSocketServer
{
		CWifiServer* WifiServer;

		ssize_t Read(char* data, ssize_t sizeOfData);

		ssize_t Send(char* data, ssize_t sizeOfData);

		TOrder GetOrder();

		void SendList();

		void ChangeCoordinate();

	public :

		CCTRLServer(CWifiServer* socketWifi);

		void ReceiveOrder();
};

#endif
