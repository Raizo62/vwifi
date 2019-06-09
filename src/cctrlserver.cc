#include "cctrlserver.h"

CCTRLServer::CCTRLServer(CWifiServer* wifiServer) : CSocketServer(AF_INET)
{
	WifiServer=wifiServer;
}

ssize_t CCTRLServer::Read(char* data, ssize_t sizeOfData)
{
		return CSocket::Read(GetSocketClient(0),data, sizeOfData);
}

ssize_t CCTRLServer::Send(char* data, ssize_t sizeOfData)
{
		return CSocket::Send(GetSocketClient(0),data, sizeOfData);
}

Order CCTRLServer::GetOrder()
{
	Order order;

	if( GetNumberClient() != 1 )
		return ORDER_NO;

	if( Read((char*)&order, sizeof(Order)) == SOCKET_ERROR )
		return ORDER_NO;

	return order;
}

void CCTRLServer::SendList()
{
	Number number=WifiServer->GetNumberClient();
	if( Send((char*)&number, sizeof(number)) == SOCKET_ERROR )
		return;

	for(Number i=0; i<number;i++)
	{
		CInfoWifi* infoWifi=WifiServer->GetReferenceOnInfoClientByNumber(i);
		if( Send((char*)infoWifi,sizeof(CInfoWifi)) == SOCKET_ERROR )
		{
			cerr<<"Error : AskList : socket.SendList : CInfoWifi : "<<*infoWifi<<endl;
			return;
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

	CInfoWifi* infoWifi;

	infoWifi=WifiServer->GetReferenceOnInfoClientByCID(cid);
	if( infoWifi == NULL )
		return;

	infoWifi->Set(coo);
}

void CCTRLServer::ReceiveOrder()
{
	if ( Accept() == SOCKET_ERROR )
		return;

	Order order=GetOrder();

	switch( order )
	{
			case ORDER_NO : break ;

			case ORDER_LIST :
				SendList();
				break;

			case ORDER_CHANGE_COORDINATE :
				ChangeCoordinate();
				break;
	}

	CloseClient(0);
}
