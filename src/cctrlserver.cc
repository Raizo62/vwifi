#include "cctrlserver.h"
#include "ccoordinate.h" // CCoordinate

CCTRLServer::CCTRLServer(CWifiServer* wifiServer) : CSocketServer(AF_INET)
{
	WifiServer=wifiServer;
}

ssize_t CCTRLServer::Read(char* data, ssize_t sizeOfData)
{
		return CSocket::Read(GetSocketClient(0),data, sizeOfData);
}

Order CCTRLServer::GetOrder()
{
	Order order;

	if( GetNumberClient() != 1 )
		return NO_ORDER;

	if( Read((char*)&order, sizeof(Order)) == SOCKET_ERROR )
		return NO_ORDER;

	return order;
}

void CCTRLServer::ChangeCoordinate()
{
	TypeCID cid;

	if( Read((char*)&cid, sizeof(TypeCID)) == SOCKET_ERROR )
		return;

	CCoordinate coo;

	if( Read((char*)&coo, sizeof(coo)) == SOCKET_ERROR )
		return;

	CInfoWifi* infoWifi;

	infoWifi=WifiServer->GetReferenceOnInfoClient(cid);
	if( infoWifi == NULL )
		return;

	infoWifi->Set(coo);
	cout<<"-> "<<*infoWifi<<endl;
}

void CCTRLServer::ReceiveOrder()
{
	if ( Accept() == SOCKET_ERROR )
		return;

	Order order=GetOrder();

	switch( order )
	{
			case NO_ORDER : break ;

			case CHANGE_COORDINATE :
				ChangeCoordinate();
				break;
	}

	CloseClient(0);
}
