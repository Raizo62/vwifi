#ifndef _CCTRLSERVER_H_
#define _CCTRLSERVER_H_

#include "cwifiserver.h" // WifiServer CSocketServer

typedef int Order;

const int ORDER_NO=0;
const int ORDER_LIST=1;
const int ORDER_CHANGE_COORDINATE=2;

class CCTRLServer : public CSocketServer
{
		CWifiServer* WifiServer;

		ssize_t Read(char* data, ssize_t sizeOfData);

		ssize_t Send(char* data, ssize_t sizeOfData);

		Order GetOrder();

		void SendList();

		void ChangeCoordinate();

	public :

		CCTRLServer(CWifiServer* socketWifi);

		void ReceiveOrder();
};

#endif
