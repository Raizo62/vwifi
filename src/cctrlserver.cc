#include "cctrlserver.h"

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

	Value x;
	Value y;
	Value z;

	if( Read((char*)&x, sizeof(Value)) == SOCKET_ERROR )
		return;
	if( Read((char*)&y, sizeof(Value)) == SOCKET_ERROR )
		return;
	if( Read((char*)&z, sizeof(Value)) == SOCKET_ERROR )
		return;

	CInfoWifi* infoWifi;

	infoWifi=WifiServer->GetReferenceOnInfoClient(cid);
	if( infoWifi == NULL )
		return;

	infoWifi->Set(x, y, z);
	cout<<*infoWifi<<endl;
}

void CCTRLServer::ReceiveOrder()
{
	Order order=GetOrder();

	switch( order )
	{
			case NO_ORDER : return ;

			case CHANGE_COORDINATE :
				ChangeCoordinate();
				return;
	}
}
