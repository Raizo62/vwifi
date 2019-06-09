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
	}

	CloseClient(0);
}
