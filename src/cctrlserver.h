#ifndef _CCTRLSERVER_H_
#define _CCTRLSERVER_H_

#include "cwifiserver.h" // CWifiServer CSocketServer
#include "cselect.h" // CSelect
#include "types.h" // TOrder

class CCTRLServer : public CSocketServer
{
		CWifiServer* WifiGuestVHostServer;
		CWifiServer* WifiGuestInetServer;
		CWifiServer* WifiHostServer;
		CSelect* Scheduler;

		ssize_t Read(char* data, ssize_t sizeOfData);

		ssize_t Send(char* data, ssize_t sizeOfData);

		TOrder GetOrder();

		void SendList();

		void ChangeCoordinate();

		void ChangePacketLoss();

		void SendStatus();

		void SendShow();

		void SendDistance();

		void CloseAllClient();

	public :

		CCTRLServer(CWifiServer* wifiGuestVHostServer, CWifiServer* wifiGuestInetServer, CWifiServer* wifiHostServer, CSelect* scheduler);

		void ReceiveOrder();
};

#endif
