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
	MaxClient=maxClient;

	if( ! CSocketServer::Listen() )
		return false;

	InfoClient = new CInfoWifi [ maxClient ];
	if( InfoClient == NULL )
	{
		perror("CWifiServer::Listen : new");
		return false;
	}

	return true;
}

bool CWifiServer::RecoverCoordinateOfOldInfoClient(TCID cid, CCoordinate& coo)
{
	for (std::list<CInfoWifi>::iterator it=InfoClientDeconnected.begin(); it != InfoClientDeconnected.end(); ++it)
	{
		if ( it->GetCid() == cid )
		{
			coo=*it;

			InfoClientDeconnected.erase(it);

			return true;
		}
	}

	return false;
}

bool CWifiServer::RecoverCoordinateOfInfoClient(TCID cid, CCoordinate& coo)
{
	for (TIndex i = 0; i < NumberClient; i++)
	{
		if( InfoClient[i].GetCid() == cid )
		{
			coo=InfoClient[i];

			InfoClient[i].DisableIt();

			return true;
		}
	}

	return false;
}

TDescriptor CWifiServer::Accept()
{
	TDescriptor new_socket;

	struct sockaddr_in address;

	new_socket = CSocketServer::Accept(address);

	if( new_socket ==  SOCKET_ERROR )
		return SOCKET_ERROR;

	TCID cid;
	//add new socket to array of sockets
	// be careful : NumberClient is already increase
	if( Type == AF_VSOCK )
		// AF_VSOCK
		cid=((struct sockaddr_vm*)&address)->svm_cid;
	else // AF_INET
		cid=ntohs(address.sin_port);

	CCoordinate coo;
	if( RecoverCoordinateOfOldInfoClient(cid,coo) || RecoverCoordinateOfInfoClient(cid, coo) )
		InfoClient[NumberClient-1].Set(coo);

	InfoClient[NumberClient-1].SetCid(cid);

	return new_socket;
}

bool CWifiServer::IsEnable(TIndex index)
{
	if( index >= NumberClient )
	{
		cerr<<"Error : CWifiServer::IsEnable : " << index <<" >= "<<NumberClient<<endl;
		return false;
	}

	return InfoClient[index].IsEnable();
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

	// save the InfoClient (the coordinate of the cid)
	if( InfoClientDeconnected.size() >= MaxClient )
		InfoClientDeconnected.pop_front();
	InfoClientDeconnected.push_back(InfoClient[index]);

	// be careful : NumberClient is already decrease
	// InfoClient : [index,NumberClient[ <-=- [index+1,NumberClient]
	if( index <  NumberClient )
		memcpy(&(InfoClient[index]),&(InfoClient[index+1]),(NumberClient-index)*sizeof(CInfoWifi));
}

void CWifiServer::SendAllOtherClients(TIndex index,const char* data, ssize_t sizeOfData)
{
//	CCoordinate coo=InfoClient[index];

	for (TIndex i = 0; i < NumberClient; i++)
	{
		if( i != index )
			if( InfoClient[i].IsEnable() )
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
