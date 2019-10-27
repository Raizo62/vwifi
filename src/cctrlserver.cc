#include "cctrlserver.h"

CCTRLServer::CCTRLServer(CWifiServer* wifiGuestServer, CWifiServer* wifiHostServer, CScheduler* scheduler) : CSocketServer(AF_INET)
{
	WifiGuestServer=wifiGuestServer;
	WifiHostServer=wifiHostServer;
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
	TIndex number=WifiGuestServer->GetNumberClient();
	if( Send((char*)&number, sizeof(number)) == SOCKET_ERROR )
		return;

	for(TIndex i=0; i<number;i++)
	{
		if( WifiGuestServer->IsEnable(i) )
		{
			CInfoWifi* infoWifi=WifiGuestServer->GetReferenceOnInfoWifiByIndex(i);
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

	infoWifi=WifiGuestServer->GetReferenceOnInfoWifiByCID(cid);
	if( infoWifi == NULL )
	{
		infoWifi=WifiGuestServer->GetReferenceOnInfoWifiDeconnectedByCID(cid);
		if( infoWifi == NULL )
		{
			CInfoWifi infoWifi(cid,coo);
			WifiGuestServer->AddInfoWifiDeconnected(infoWifi);
			return;
		}
	}

	infoWifi->Set(coo);
}

void CCTRLServer::ChangePacketLoss()
{
	int value;

	if( Read((char*)&value, sizeof(value)) == SOCKET_ERROR )
		return;

	if ( value )
	{
		#ifdef _DEBUG
			cout<<"Packet loss : Enable"<<endl;
		#endif
		WifiGuestServer->SetPacketLoss(true);
	}
	else
	{
		#ifdef _DEBUG
			cout<<"Packet loss : Disable"<<endl;
		#endif
		WifiGuestServer->SetPacketLoss(false);
	}
}

void CCTRLServer::SendStatus()
{
	if( Send((char*)&(WifiGuestServer->Type),sizeof(WifiGuestServer->Type)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : Type"<<endl;
		return;
	}

	if( Send((char*)&(WifiGuestServer->Port),sizeof(WifiGuestServer->Port)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : Port"<<endl;
		return;
	}

	if( Send((char*)&(WifiGuestServer->MaxClientDeconnected),sizeof(WifiGuestServer->MaxClientDeconnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : Size"<<endl;
		return;
	}

	if( Send((char*)&(WifiGuestServer->PacketLoss),sizeof(WifiGuestServer->PacketLoss)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : PacketLoss"<<endl;
		return;
	}
}

void CCTRLServer::SendShow()
{
	if( Send((char*)&(WifiGuestServer->PacketLoss),sizeof(WifiGuestServer->PacketLoss)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : PacketLoss"<<endl;
		return;
	}
}

void CCTRLServer::CloseAllClient()
{
	// be careful : In the Scheduler, i must delete only the nodes of Wifi Guest, not the node of the CTRLServer
	for (TIndex i = 0; i < WifiGuestServer->GetNumberClient(); i++)
		Scheduler->DelNode((*WifiGuestServer)[i]);

	WifiGuestServer->CloseAllClient();
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

			case TORDER_PACKET_LOSS :
				ChangePacketLoss();
				break;

			case TORDER_STATUS :
				SendStatus();
				break;

			case TORDER_SHOW :
				SendShow();
				break;

			case TORDER_CLOSE_ALL_CLIENT :
				CloseAllClient();
				break;
	}

	CloseClient(0);
}
