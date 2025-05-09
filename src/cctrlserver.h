#ifndef _CCTRLSERVER_H_
#define _CCTRLSERVER_H_

#include "cwifiserver.h" // CWifiServer CSocketServer
#include "csocketserverfunctionitcp.h" // CSocketServerFunctionITCP
#include "cselect.h" // CSelect
#include "types.h" // TOrder

class CCTRLServer : public CSocketServer, public CSocketServerFunctionITCP
{
		CWifiServer* WifiServerVTCP;
		CWifiServer* WifiServerITCP;
		CWifiServer* WifiServerSPY;
		CSelect* Scheduler;

		bool _Listen(TDescriptor& master, TPort port) override;
		TDescriptor _Accept(TDescriptor master, TCID& cid) override;

		using CSocketServer::Read;
		ssize_t Read(char* data, ssize_t sizeOfData);

		using CSocketServer::Send;
		ssize_t Send(char* data, ssize_t sizeOfData);

		TOrder GetOrder();

		bool SendCInfoWifi(CInfoWifi* infoWifi);

		void SendList();

		void ChangeCoordinate();

		void SetName();

		void ChangePacketLoss();

		void SendStatus();

		void SendShow();

		void SendDistance();

		void SetScale();

		void CloseAllClient();

	public :

		CCTRLServer(CWifiServer* wifiServerVTCP, CWifiServer* wifiServerITCP, CWifiServer* wifiServerSPY, CSelect* scheduler);

		virtual ~CCTRLServer();

		void ReceiveOrder();
};

#endif
