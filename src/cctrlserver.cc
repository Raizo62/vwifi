#include <cstring> // strcpy

#include "cctrlserver.h"
#include "config.h" // MAX_SIZE_NAME


CCTRLServer::CCTRLServer(CWifiServer* wifiServerVTCP, CWifiServer* wifiServerITCP, CWifiServer* wifiServerSPY, CSelect* scheduler) : CSocketServer()
{
	WifiServerVTCP=wifiServerVTCP;
	WifiServerITCP=wifiServerITCP;
	WifiServerSPY=wifiServerSPY;
	Scheduler=scheduler;
}

CCTRLServer::~CCTRLServer()
{
	// Close any open connections
	CloseAllClient();
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

	if( Read(reinterpret_cast<char*>(&order), sizeof(TOrder)) == SOCKET_ERROR )
		return TORDER_NO;

	return order;
}

bool CCTRLServer::SendCInfoWifi(CInfoWifi* infoWifi)
{
	TCID cid=infoWifi->GetCid();
	if( Send(reinterpret_cast<char*>(&cid),sizeof(cid)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendCInfoWifi : cid : "<<infoWifi->GetCid()<<endl;
		return false;
	}

	CCoordinate coo=(*infoWifi);
	if( Send(reinterpret_cast<char*>(&coo),sizeof(coo)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendCInfoWifi : CCoordinate : "<<infoWifi->GetCid()<<endl;
		return false;
	}

	int sizeName=infoWifi->GetSizeName();
	if( Send(reinterpret_cast<char*>(&sizeName),sizeof(sizeName)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendCInfoWifi : size of name : "<<infoWifi->GetCid()<<endl;
		return false;
	}
	if( sizeName > 0 )
	{
		char name[MAX_SIZE_NAME+1]; // +1 : \0
		strcpy(name,(infoWifi->GetName()).c_str());
		if( Send(name,sizeName+1) == SOCKET_ERROR ) // +1 : \0
		{
			cerr<<"Error : SendCInfoWifi : name : "<<infoWifi->GetCid()<<endl;
			return false;
		}
	}

	return true;
}

void CCTRLServer::SendList()
{
	// because the same List is shared by WifiServerVTCP and WifiServerITCP

	CInfoWifi* infoWifi;

	// Spies :

	TIndex number=WifiServerSPY->GetNumberClient();

	if( Send(reinterpret_cast<char*>(&number), sizeof(number)) == SOCKET_ERROR )
		return;

	for(TIndex i=0; i<number;i++)
	{
		if( WifiServerSPY->IsEnable(i) )
		{
			infoWifi=WifiServerSPY->GetReferenceOnInfoWifiByIndex(i);
			if( ! SendCInfoWifi(infoWifi) )
			{
				cerr<<"Error : SendList : Send : Spies : CInfoWifi : "<<*infoWifi<<endl;
				return;
			}
		}
	}

	// Clients

	number=WifiServerVTCP->GetNumberClient();

	if( Send(reinterpret_cast<char*>(&number), sizeof(number)) == SOCKET_ERROR )
		return;

	for(TIndex i=0; i<number;i++)
	{
		if( WifiServerVTCP->IsEnable(i) )
		{
			infoWifi=WifiServerVTCP->GetReferenceOnInfoWifiByIndex(i);
			if( ! SendCInfoWifi(infoWifi) )
			{
				cerr<<"Error : SendList : Send : Clients : CInfoWifi : "<<*infoWifi<<endl;
				return;
			}
		}
	}
}

void CCTRLServer::ChangeCoordinate()
{
	TCID cid;

	if( Read(reinterpret_cast<char*>(&cid), sizeof(TCID)) == SOCKET_ERROR )
		return;

	CCoordinate coo;

	if( Read(reinterpret_cast<char*>(&coo), sizeof(coo)) == SOCKET_ERROR )
		return;

	if( cid < TCID_GUEST_MIN )
		return;

	// because the same List is shared by WifiServerVTCP and WifiServerITCP

	CInfoWifi* infoWifi;

	infoWifi=WifiServerVTCP->GetReferenceOnInfoWifiByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->Set(coo);
		return;
	}

	infoWifi=WifiServerVTCP->GetReferenceOnInfoWifiDeconnectedByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->Set(coo);
		return;
	}

	CInfoWifi infoNewWifi(cid,coo);
	WifiServerVTCP->AddInfoWifiDeconnected(infoNewWifi);
}

void CCTRLServer::SetName()
{
	TCID cid;

	if( Read(reinterpret_cast<char*>(&cid), sizeof(TCID)) == SOCKET_ERROR )
		return;

	int sizeName;
	char strName[MAX_SIZE_NAME+1]; // +1 : \0

	if( Read(reinterpret_cast<char*>(&sizeName), sizeof(sizeName)) == SOCKET_ERROR )
		return;

	if( Read(reinterpret_cast<char*>(strName), sizeof(strName)) == SOCKET_ERROR )
		return;

	if( cid < TCID_GUEST_MIN )
		return;

	// because the same List is shared by WifiServerVTCP and WifiServerITCP

	string name(strName);

	CInfoWifi* infoWifi;

	infoWifi=WifiServerVTCP->GetReferenceOnInfoWifiByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->SetName(name);
		return;
	}

	infoWifi=WifiServerVTCP->GetReferenceOnInfoWifiDeconnectedByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->SetName(name);
		return;
	}

	infoWifi=WifiServerSPY->GetReferenceOnInfoWifiByCID(cid);
	if( infoWifi != NULL )
	{
		infoWifi->SetName(name);
		return;
	}
}

void CCTRLServer::ChangePacketLoss()
{
	int value;

	if( Read(reinterpret_cast<char*>(&value), sizeof(value)) == SOCKET_ERROR )
		return;

	if ( value )
	{
		#ifdef _DEBUG
			cout<<"Packet loss : Enable"<<endl;
		#endif
		CanLostPackets=true;
	}
	else
	{
		#ifdef _DEBUG
			cout<<"Packet loss : Disable"<<endl;
		#endif
		CanLostPackets=false;
	}
}

void CCTRLServer::SendStatus()
{
	if( Send(reinterpret_cast<char*>(&CanLostPackets),sizeof(CanLostPackets)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : PacketLoss"<<endl;
		return;
	}

	if( Send(reinterpret_cast<char*>(&Scale),sizeof(Scale)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : Scale"<<endl;
		return;
	}

	// VHOST

	if( Send(reinterpret_cast<char*>(&WifiServerVTCP->Port),sizeof(WifiServerVTCP->Port)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : Port VHOST"<<endl;
		return;
	}

	// INET

	if( Send(reinterpret_cast<char*>(&WifiServerITCP->Port),sizeof(WifiServerITCP->Port)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : Port INET"<<endl;
		return;
	}

	// SizeOfDisconnected
	// be careful : the same List is shared by WifiServerVTCP and WifiServerITCP

	if( Send(reinterpret_cast<char*>(&WifiServerVTCP->MaxClientDeconnected),sizeof(WifiServerVTCP->MaxClientDeconnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : Size MaxClientDeconnected"<<endl;
		return;
	}

	// SPY

	bool spyIsConnected=( WifiServerSPY->GetNumberClient() > 0 );
	if( Send(reinterpret_cast<char*>(&spyIsConnected),sizeof(spyIsConnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendStatus : Send : spyIsConnected"<<endl;
		return;
	}
}

void CCTRLServer::SendShow()
{
	if( Send(reinterpret_cast<char*>(&CanLostPackets),sizeof(CanLostPackets)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendShow : Send : PacketLoss"<<endl;
		return;
	}

	if( Send(reinterpret_cast<char*>(&Scale),sizeof(Scale)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendShow : Send : Scale"<<endl;
		return;
	}

	bool spyIsConnected=( WifiServerSPY->GetNumberClient() > 0 );
	if( Send(reinterpret_cast<char*>(&spyIsConnected),sizeof(spyIsConnected)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendShow : Send : spyIsConnected"<<endl;
		return;
	}
}

void CCTRLServer::SendDistance()
{
	TCID cid1, cid2;

	if( Read(reinterpret_cast<char*>(&cid1), sizeof(TCID)) == SOCKET_ERROR )
		return;

	if( Read(reinterpret_cast<char*>(&cid2), sizeof(TCID)) == SOCKET_ERROR )
		return;

	int codeError;

	// because the same List is shared by WifiServerVTCP and WifiServerITCP

	CCoordinate* coo1;

	coo1=WifiServerVTCP->GetReferenceOnInfoWifiByCID(cid1);
	if( coo1 == NULL )
	{
		coo1=WifiServerVTCP->GetReferenceOnInfoWifiDeconnectedByCID(cid1);
		if( coo1 == NULL )
		{
			codeError=-1;
			if( Send(reinterpret_cast<char*>(&codeError),sizeof(codeError)) == SOCKET_ERROR )
				cerr<<"Error : SendDistance : Send : unknown cid1"<<endl;
			return ;
		}
	}

	CCoordinate* coo2;

	coo2=WifiServerVTCP->GetReferenceOnInfoWifiByCID(cid2);
	if( coo2 == NULL )
	{
		coo2=WifiServerVTCP->GetReferenceOnInfoWifiDeconnectedByCID(cid2);
		if( coo2 == NULL )
		{
			codeError=-2;
			if( Send(reinterpret_cast<char*>(&codeError),sizeof(codeError)) == SOCKET_ERROR )
				cerr<<"Error : SendDistance : Send : unknown cid2"<<endl;
			return ;
		}
	}

	codeError=0;
	if( Send(reinterpret_cast<char*>(&codeError),sizeof(codeError)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendDistance : Send : no error"<<endl;
		return ;
	}

	TDistance distance=coo1->DistanceWith(*coo2);

	if( Send(reinterpret_cast<char*>(&distance),sizeof(distance)) == SOCKET_ERROR )
	{
		cerr<<"Error : SendDistance : Send : distance"<<endl;
		return;
	}
}

void CCTRLServer::SetScale()
{
	TScale new_scale;

	if( Read(reinterpret_cast<char*>(&new_scale), sizeof(new_scale)) == SOCKET_ERROR )
		return;

	Scale=new_scale;
}

void CCTRLServer::CloseAllClient()
{
	// because the same List is shared by WifiServerVTCP and WifiServerITCP

	// Clients :

	// be careful : In the Scheduler, i must delete only the nodes of WifiServer, not the node of the CTRLServer
	for (TIndex i = 0; i < WifiServerVTCP->GetNumberClient(); i++)
		Scheduler->DelNode((*WifiServerVTCP)[i]);

	WifiServerVTCP->CloseAllClient();

	// Spies :

	for (TIndex i = 0; i < WifiServerSPY->GetNumberClient(); i++)
		Scheduler->DelNode((*WifiServerSPY)[i]);

	WifiServerSPY->CloseAllClient();
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

			case TORDER_SETNAME :
				SetName();
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

			case TORDER_SET_SCALE :
				SetScale();
				break;

			case TORDER_CLOSE_ALL_CLIENT :
				CloseAllClient();
				break;
	}

	CloseClient(0);
}
