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

ssize_t CCTRLServer::Send(char* data, ssize_t sizeOfData)
{
		return CSocket::Send(GetSocketClient(0),data, sizeOfData);
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

void CCTRLServer::SendList()
{
	unsigned int number=WifiServer->GetNumberClient();
	if( Send((char*)&number, sizeof(number)) == SOCKET_ERROR )
		return;

	for(unsigned int i=0; i<number;i++)
	{
		CInfoWifi* infoWifi=WifiServer->GetReferenceOnInfoClientByNumber(i);
		if( Send((char*)infoWifi,sizeof(CInfoWifi)) == SOCKET_ERROR )
		{
			cerr<<"Error : AskList : socket.SendList : CInfoWifi : "<<*infoWifi<<endl;
			return;
		}
		cout<<*infoWifi<<endl;
	}
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

	infoWifi=WifiServer->GetReferenceOnInfoClientByCID(cid);
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

			case ORDER_LIST :
				SendList();
				break;

			case CHANGE_COORDINATE :
				ChangeCoordinate();
				break;
	}

	CloseClient(0);
}
