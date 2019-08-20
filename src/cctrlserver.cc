#include "cctrlserver.h"

CCTRLServer::CCTRLServer(CWifiServer* wifiServer, CScheduler* scheduler) : CSocketServer(AF_INET)
{
	WifiServer=wifiServer;
	Scheduler=scheduler;
}

ssize_t CCTRLServer::Read(char* data, ssize_t sizeOfData)
{
		return CSocket::Read(GetSocketClient(0),data, sizeOfData);
}

ssize_t CCTRLServer::Send(char* data, ssize_t sizeOfData)
{
		return CSocket::Send(GetSocketClient(0),data, sizeOfData);
}

TOrder CCTRLServer::GetOrder()
{
	TOrder order;

	if( GetNumberClient() != 1 )
		return TORDER_NO;

	if( Read((char*)&order, sizeof(TOrder)) == SOCKET_ERROR )
		return TORDER_NO;

	return order;
}

void CCTRLServer::SendList()
{
	TIndex number=WifiServer->GetNumberClient();
	if( Send((char*)&number, sizeof(number)) == SOCKET_ERROR )
		return;

	for(TIndex i=0; i<number;i++)
	{
		if( WifiServer->IsEnable(i) )
		{
			CInfoWifi* infoWifi=WifiServer->GetReferenceOnInfoWifiByIndex(i);
			if( Send((char*)infoWifi,sizeof(CInfoWifi)) == SOCKET_ERROR )
			{
				cerr<<"Error : AskList : socket.SendList : CInfoWifi : "<<*infoWifi<<endl;
				return;
			}
		}
	}
}

void CCTRLServer::ChangeCoordinate()
{
	TCID cid;

	if( Read((char*)&cid, sizeof(TCID)) == SOCKET_ERROR )
		return;

	CCoordinate coo;

	if( Read((char*)&coo, sizeof(coo)) == SOCKET_ERROR )
		return;

	if( cid < TCID_GUEST_MIN )
		return;

	CInfoWifi* infoWifi;

	infoWifi=WifiServer->GetReferenceOnInfoWifiByCID(cid);
	if( infoWifi == NULL )
	{
		infoWifi=WifiServer->GetReferenceOnInfoWifiDeconnectedByCID(cid);
		if( infoWifi == NULL )
		{
			CInfoWifi infoWifi(cid,coo);
			WifiServer->AddInfoWifiDeconnected(infoWifi);
			return;
		}
	}

	infoWifi->Set(coo);
}

void CCTRLServer::CloseAllClient()
{
	// be careful : In the Scheduler, i must delete only the nodes of Wifi Guest, not the node of the CTRLServer
	for (TIndex i = 0; i < WifiServer->GetNumberClient(); i++)
		Scheduler->DelNode((*WifiServer)[i]);

	WifiServer->CloseAllClient();
}

void CCTRLServer::ReceiveOrder()
{
	if ( Accept() == SOCKET_ERROR )
		return;

	TOrder order=GetOrder();

	switch( order )
	{
			case TORDER_NO : break ;

			case TORDER_LIST :
				SendList();
				break;

			case TORDER_CHANGE_COORDINATE :
				ChangeCoordinate();
				break;

			case TORDER_CLOSE_ALL_CLIENT :
				CloseAllClient();
				break;
	}

	CloseClient(0);
}
