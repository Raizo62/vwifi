#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy

#include <arpa/inet.h> // struct sockaddr_in
#include <sys/socket.h> // AF_VSOCK / AF_INET
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "cwifiserver.h"

using namespace std;

bool CWifiServer::Listen(unsigned int maxClient)
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

Descriptor CWifiServer::Accept()
{
	Descriptor new_socket;

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

void CWifiServer::ShowInfoClient(unsigned int number)
{
	if( number >= NumberClient )
	{
		cerr<<"Error : CWifiServer::ShowInfoClient : " << number <<" >= "<<NumberClient<<endl;
		return;
	}

	cout<<"{"<<InfoClient[number]<<"}";
}

void CWifiServer::CloseClient(unsigned int number)
{
	if( number >= NumberClient )
		return;

	CSocketServer::CloseClient(number);

	// be careful : NumberClient is already decrease
	// InfoClient : [number,NumberClient[ <-=- [number+1,NumberClient]
	if( number <  NumberClient )
		memcpy(&(InfoClient[number]),&(InfoClient[number+1]),(NumberClient-number)*sizeof(CInfoWifi));
}

void CWifiServer::SendAllOtherClients(unsigned int number,const char* data, ssize_t sizeOfData)
{
	CCoordinate coo=InfoClient[number];

	for (unsigned int i = 0; i < NumberClient; i++)
	{
		if( i != number )
			if ( coo.DistanceWith(InfoClient[i]) < 10 )
				Send(SocketClients[i], data, sizeOfData);
	}
}

CInfoWifi* CWifiServer::GetReferenceOnInfoClientByCID(TypeCID cid)
{
	for (unsigned int i = 0; i < NumberClient; i++)
	{
		if( InfoClient[i].GetCid() == cid )
			return &(InfoClient[i]);
	}

	return NULL;
}

CInfoWifi* CWifiServer::GetReferenceOnInfoClientByNumber(unsigned int number)
{
	if( number >= NumberClient )
	{
		cerr<<"Error : CWifiServer::GetReferenceOnInfoClientByNumber : " << number <<" >= "<<NumberClient<<endl;
		return NULL;
	}

	return &(InfoClient[number]);
}
