#include "cctrlserver.h"

CCTRLServer::CCTRLServer(CWifiServer* wifiGuestVHostServer, CWifiServer* wifiGuestInetServer, CWifiServer* wifiHostServer, CScheduler* scheduler) : CSocketServer(AF_INET)
{
	WifiGuestVHostServer=wifiGuestVHostServer;
	WifiGuestInetServer=wifiGuestInetServer;
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
	TIndex numberVHost=WifiGuestVHostServer->GetNumberClient();
	TIndex numberInet=WifiGuestInetServer->GetNumberClient();
	TIndex number=numberVHost+numberInet;
	if( Send((char*)&number, sizeof(number)) == SOCKET_ERROR )
		return;

	for(TIndex i=0; i<numberVHost;i++)
	{
		if( WifiGuestVHostServer->IsEnable(i) )
		{
			CInfoWifi* infoWifi=WifiGuestVHostServer->GetReferenceOnInfoWifiByIndex(i);
			if( Send((char*)infoWifi,sizeof(CInfoWifi)) == SOCKET_ERROR )
			{
				cerr<<"Error : AskList : socket.SendList : CInfoWifi : "<<*infoWifi<<endl;
				return;
			}
		}
	}
	for(TIndex i=0; i<numberInet;i++)
	{
		if( WifiGuestInetServer->IsEnable(i) )
		{
			CInfoWifi* infoWifi=WifiGuestInetServer->GetReferenceOnInfoWifiByIndex(i);
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

	infoWifi=WifiGuestVHostServer->GetReferenceOnInfoWifiByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->Set(coo);
		return;
	}

	infoWifi=WifiGuestVHostServer->GetReferenceOnInfoWifiDeconnectedByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->Set(coo);
		return;
	}

	infoWifi=WifiGuestInetServer->GetReferenceOnInfoWifiByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->Set(coo);
		return;
	}

	infoWifi=WifiGuestInetServer->GetReferenceOnInfoWifiDeconnectedByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->Set(coo);
		return;
	}

	CInfoWifi infoNewWifi(cid,coo);
#ifdef _USE_VSOCK_BY_DEFAULT_
	WifiGuestVHostServer->AddInfoWifiDeconnected(infoNewWifi);
#else
	WifiGuestInetServer->AddInfoWifiDeconnected(infoNewWifi);
#endif

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
		WifiGuestVHostServer->SetPacketLoss(true);
		WifiGuestInetServer->SetPacketLoss(true);
	}
	else
	{
		#ifdef _DEBUG
			cout<<"Packet loss : Disable"<<endl;
		#endif
		WifiGuestVHostServer->SetPacketLoss(false);
		WifiGuestInetServer->SetPacketLoss(false);
	}
}

void CCTRLServer::SendStatus()
{
	if( Send((char*)&(WifiGuestVHostServer->PacketLoss),sizeof(WifiGuestVHostServer->PacketLoss)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : PacketLoss"<<endl;
		return;
	}

	// VHOST

	if( Send((char*)&(WifiGuestVHostServer->Port),sizeof(WifiGuestVHostServer->Port)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : Port"<<endl;
		return;
	}

	if( Send((char*)&(WifiGuestVHostServer->MaxClientDeconnected),sizeof(WifiGuestVHostServer->MaxClientDeconnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : Size"<<endl;
		return;
	}

	// INET

	if( Send((char*)&(WifiGuestInetServer->Port),sizeof(WifiGuestInetServer->Port)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : Port"<<endl;
		return;
	}

	if( Send((char*)&(WifiGuestInetServer->MaxClientDeconnected),sizeof(WifiGuestInetServer->MaxClientDeconnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : Size"<<endl;
		return;
	}

	// HOST

	bool hostIsConnected=( WifiHostServer->GetNumberClient() > 0 );
	if( Send((char*)&hostIsConnected,sizeof(hostIsConnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : socket.SendList : HostIsConnected"<<endl;
		return;
	}

}

void CCTRLServer::SendShow()
{
	if( Send((char*)&(WifiGuestVHostServer->PacketLoss),sizeof(WifiGuestVHostServer->PacketLoss)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendShow : socket.SendList : PacketLoss"<<endl;
		return;
	}

	bool hostIsConnected=( WifiGuestVHostServer->GetNumberClient() > 0 );
	if( Send((char*)&hostIsConnected,sizeof(hostIsConnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendShow : socket.SendList : HostIsConnected"<<endl;
		return;
	}
}

void CCTRLServer::CloseAllClient()
{
	// be careful : In the Scheduler, i must delete only the nodes of Wifi Guest, not the node of the CTRLServer
	for (TIndex i = 0; i < WifiGuestVHostServer->GetNumberClient(); i++)
		Scheduler->DelNode((*WifiGuestVHostServer)[i]);

	for (TIndex i = 0; i < WifiGuestInetServer->GetNumberClient(); i++)
		Scheduler->DelNode((*WifiGuestInetServer)[i]);

	WifiGuestVHostServer->CloseAllClient();
	WifiGuestInetServer->CloseAllClient();
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
