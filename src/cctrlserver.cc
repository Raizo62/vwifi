#include "cctrlserver.h"

CCTRLServer::CCTRLServer(CWifiServer* wifiServerVTCP, CWifiServer* wifiServerITCP, CWifiServer* wifiServerSPY, CSelect* scheduler) : CSocketServer()
{
	WifiServerVTCP=wifiServerVTCP;
	WifiServerITCP=wifiServerITCP;
	WifiServerSPY=wifiServerSPY;
	Scheduler=scheduler;
}

bool CCTRLServer::_Listen(TDescriptor& master, TPort port)
{
	return CSocketServerFunctionITCP::_Listen(master, port);
}

TDescriptor CCTRLServer::_Accept(TDescriptor master, TCID& cid)
{
	return CSocketServerFunctionITCP::_Accept(master, cid);
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
	// because the same List is shared by WifiServerVTCP and WifiServerITCP
	CWifiServer* wifiServer=WifiServerVTCP;

	TIndex number=wifiServer->GetNumberClient();

	if( Send((char*)&number, sizeof(number)) == SOCKET_ERROR )
		return;

	for(TIndex i=0; i<number;i++)
	{
		if( wifiServer->IsEnable(i) )
		{
			CInfoWifi* infoWifi=wifiServer->GetReferenceOnInfoWifiByIndex(i);
			if( Send((char*)infoWifi,sizeof(CInfoWifi)) == SOCKET_ERROR )
			{
				cerr<<"Error : SendList : Send : CInfoWifi : "<<*infoWifi<<endl;
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

	// because the same List is shared by WifiServerVTCP and WifiServerITCP
	CWifiServer* wifiServer=WifiServerVTCP;

	CInfoWifi* infoWifi;

	infoWifi=wifiServer->GetReferenceOnInfoWifiByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->Set(coo);
		return;
	}

	infoWifi=wifiServer->GetReferenceOnInfoWifiDeconnectedByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->Set(coo);
		return;
	}

	CInfoWifi infoNewWifi(cid,coo);
	WifiServerVTCP->AddInfoWifiDeconnected(infoNewWifi);
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
		WifiServerVTCP->SetPacketLoss(true);
		WifiServerITCP->SetPacketLoss(true);
	}
	else
	{
		#ifdef _DEBUG
			cout<<"Packet loss : Disable"<<endl;
		#endif
		WifiServerVTCP->SetPacketLoss(false);
		WifiServerITCP->SetPacketLoss(false);
	}
}

void CCTRLServer::SendStatus()
{
	if( Send((char*)&(WifiServerVTCP->PacketLoss),sizeof(WifiServerVTCP->PacketLoss)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : PacketLoss"<<endl;
		return;
	}

	// VHOST

	if( Send((char*)&(WifiServerVTCP->Port),sizeof(WifiServerVTCP->Port)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : Port VHOST"<<endl;
		return;
	}

	// INET

	if( Send((char*)&(WifiServerITCP->Port),sizeof(WifiServerITCP->Port)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : Port INET"<<endl;
		return;
	}

	// SizeOfDisconnected
	// be careful : the same List is shared by WifiServerVTCP and WifiServerITCP

	if( Send((char*)&(WifiServerITCP->MaxClientDeconnected),sizeof(WifiServerITCP->MaxClientDeconnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : Size MaxClientDeconnected"<<endl;
		return;
	}

	// SPY

	bool spyIsConnected=( WifiServerSPY->GetNumberClient() > 0 );
	if( Send((char*)&spyIsConnected,sizeof(spyIsConnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : spyIsConnected"<<endl;
		return;
	}

}

void CCTRLServer::SendShow()
{
	if( Send((char*)&(WifiServerVTCP->PacketLoss),sizeof(WifiServerVTCP->PacketLoss)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendShow : Send : PacketLoss"<<endl;
		return;
	}

	bool spyIsConnected=( WifiServerSPY->GetNumberClient() > 0 );
	if( Send((char*)&spyIsConnected,sizeof(spyIsConnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendShow : Send : spyIsConnected"<<endl;
		return;
	}
}

void CCTRLServer::SendDistance()
{
	TCID cid1, cid2;

	if( Read((char*)&cid1, sizeof(TCID)) == SOCKET_ERROR )
		return;

	if( Read((char*)&cid2, sizeof(TCID)) == SOCKET_ERROR )
		return;

	int codeError;

	// because the same List is shared by WifiServerVTCP and WifiServerITCP
	CWifiServer* wifiServer=WifiServerVTCP;

	CCoordinate* coo1;

	coo1=wifiServer->GetReferenceOnInfoWifiByCID(cid1);
	if( coo1 == NULL )
	{
		coo1=wifiServer->GetReferenceOnInfoWifiDeconnectedByCID(cid1);
		if( coo1 == NULL )
		{
			codeError=-1;
			if( Send((char*)&codeError,sizeof(codeError)) == SOCKET_ERROR )
				cerr<<"Error : SendDistance : Send : unknown cid1"<<endl;
			return ;
		}
	}

	CCoordinate* coo2;

	coo2=wifiServer->GetReferenceOnInfoWifiByCID(cid2);
	if( coo2 == NULL )
	{
		coo2=wifiServer->GetReferenceOnInfoWifiDeconnectedByCID(cid2);
		if( coo2 == NULL )
		{
			codeError=-2;
			if( Send((char*)&codeError,sizeof(codeError)) == SOCKET_ERROR )
				cerr<<"Error : SendDistance : Send : unknown cid2"<<endl;
			return ;
		}
	}

	codeError=0;
	if( Send((char*)&codeError,sizeof(codeError)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendDistance : Send : no error"<<endl;
		return ;
	}

	TDistance distance=coo1->DistanceWith(*coo2);

	if( Send((char*)&distance,sizeof(distance)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendDistance : Send : distance"<<endl;
		return;
	}
}

void CCTRLServer::CloseAllClient()
{
	// because the same List is shared by WifiServerVTCP and WifiServerITCP
	CWifiServer* wifiServer=WifiServerVTCP;

	// be careful : In the Scheduler, i must delete only the nodes of WifiServer, not the node of the CTRLServer
	for (TIndex i = 0; i < wifiServer->GetNumberClient(); i++)
		Scheduler->DelNode((*wifiServer)[i]);

	wifiServer->CloseAllClient();
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

			case TORDER_DISTANCE_BETWEEN_CID :
				SendDistance();
				break;

			case TORDER_CLOSE_ALL_CLIENT :
				CloseAllClient();
				break;
	}

	CloseClient(0);
}
