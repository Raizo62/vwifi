#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy

#include <arpa/inet.h> // struct sockaddr_in
#include <sys/socket.h> // AF_VSOCK / AF_INET
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "cwifiserver.h"

using namespace std;

bool CWifiServer::Listen(TIndex maxClient)
{
	if( ! CSocketServer::Listen(maxClient) )
		return false;

	InfoClient = new CInfoWifi [ maxClient ];
	if( InfoClient == NULL )
	{
		perror("CWifiServer::Listen : new");
		return false;
	}

	return true;
}

TDescriptor CWifiServer::Accept()
{
	TDescriptor new_socket;

	struct sockaddr_in address;

	new_socket = CSocketServer::Accept(address);

	if( new_socket ==  SOCKET_ERROR )
		return SOCKET_ERROR;

	//add new socket to array of sockets
	// be careful : NumberClient is already increase
	switch ( Type )
	{
		case AF_VSOCK :
			{
				InfoClient[NumberClient-1].SetCid(((struct sockaddr_vm*)&address)->svm_cid);
				break ;
			}

		case AF_INET :
			{
				InfoClient[NumberClient-1].SetCid(ntohs(address.sin_port));
				break;
			}
	}

	return new_socket;
}

void CWifiServer::ShowInfoClient(TIndex index)
{
	if( index >= NumberClient )
	{
		cerr<<"Error : CWifiServer::ShowInfoClient : " << index <<" >= "<<NumberClient<<endl;
		return;
	}

	cout<<"{"<<InfoClient[index]<<"}";
}

void CWifiServer::CloseClient(TIndex index)
{
	if( index >= NumberClient )
		return;

	CSocketServer::CloseClient(index);

	// be careful : NumberClient is already decrease
	// InfoClient : [index,NumberClient[ <-=- [index+1,NumberClient]
	if( index <  NumberClient )
		memcpy(&(InfoClient[index]),&(InfoClient[index+1]),(NumberClient-index)*sizeof(CInfoWifi));
}

void CWifiServer::SendAllOtherClients(TIndex index,const char* data, ssize_t sizeOfData)
{
	CCoordinate coo=InfoClient[index];

	for (TIndex i = 0; i < NumberClient; i++)
	{
		if( i != index )
			if ( coo.DistanceWith(InfoClient[i]) < 10 )
				Send(SocketClients[i], data, sizeOfData);
	}
}

CInfoWifi* CWifiServer::GetReferenceOnInfoClientByCID(TCID cid)
{
	for (TIndex i = 0; i < NumberClient; i++)
	{
		if( InfoClient[i].GetCid() == cid )
			return &(InfoClient[i]);
	}

	return NULL;
}

CInfoWifi* CWifiServer::GetReferenceOnInfoClientByIndex(TIndex index)
{
	if( index >= NumberClient )
	{
		cerr<<"Error : CWifiServer::GetReferenceOnInfoClientByIndex : " << index <<" >= "<<NumberClient<<endl;
		return NULL;
	}

	return &(InfoClient[index]);
}
