#ifndef _CCTRLSERVER_
#define _CCTRLSERVER_

#include "cwifiserver.h" // WifiServer CSocketServer

typedef int Order;

const int NO_ORDER=0;
const int CHANGE_COORDINATE=1;

class CCTRLServer : public CSocketServer
{
		CWifiServer* WifiServer;

		ssize_t Read(char* data, ssize_t sizeOfData);

		Order GetOrder();

		void ChangeCoordinate();

	public :

		CCTRLServer(CWifiServer* socketWifi);

		void ReceiveOrder();
};

#endif
