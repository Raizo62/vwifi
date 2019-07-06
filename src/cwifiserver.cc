#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy

#include <arpa/inet.h> // struct sockaddr_in
#include <sys/socket.h> // AF_VSOCK / AF_INET
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "cwifiserver.h"

using namespace std;

bool CWifiServer::Listen(TIndex maxClientDeconnected)
{
	MaxClientDeconnected=maxClientDeconnected;

	if( ! CSocketServer::Listen() )
		return false;

	return true;
}

bool CWifiServer::RecoverCoordinateOfOldInfoClient(TCID cid, CCoordinate& coo)
{
	for (list<CInfoWifi>::iterator it=InfoClientsDeconnected.begin(); it != InfoClientsDeconnected.end(); ++it)
	{
		if ( it->GetCid() == cid )
		{
			coo=*it;

			InfoClientsDeconnected.erase(it);

			return true;
		}
	}

	return false;
}

bool CWifiServer::RecoverCoordinateOfInfoClient(TCID cid, CCoordinate& coo)
{
	for (TIndex i = 0; i < InfoClients.size() ; i++)
	{
		if( InfoClients[i].GetCid() == cid )
		{
			coo=InfoClients[i];

			InfoClients[i].DisableIt();

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

	CInfoWifi infoWifi;

	CCoordinate coo;
	if( RecoverCoordinateOfOldInfoClient(cid,coo) || RecoverCoordinateOfInfoClient(cid, coo) )
		infoWifi.Set(coo);

	infoWifi.SetCid(cid);

	InfoClients.push_back(infoWifi);

	return new_socket;
}

bool CWifiServer::IsEnable(TIndex index)
{
	if( index >= GetNumberClient() )
	{
		cerr<<"Error : CWifiServer::IsEnable : " << index <<" >= "<<GetNumberClient()<<endl;
		return false;
	}

	return InfoClients[index].IsEnable();
}

void CWifiServer::ShowInfoClient(TIndex index)
{
	if( index >= GetNumberClient() )
	{
		cerr<<"Error : CWifiServer::ShowInfoClient : " << index <<" >= "<<GetNumberClient()<<endl;
		return;
	}

	cout<<"{"<<InfoClients[index]<<"}";
}

void CWifiServer::CloseClient(TIndex index)
{
	if( index >= GetNumberClient() )
		return;

	CSocketServer::CloseClient(index);

	// save the InfoClient (the coordinate of the cid)
	if( InfoClientsDeconnected.size() >= MaxClientDeconnected )
		InfoClientsDeconnected.pop_front();
	InfoClientsDeconnected.push_back(InfoClients[index]);

	InfoClients.erase (InfoClients.begin()+index);
}

void CWifiServer::SendAllOtherClients(TIndex index,const char* data, ssize_t sizeOfData)
{
//	CCoordinate coo=InfoClients[index];

	for (TIndex i = 0; i < GetNumberClient(); i++)
	{
		if( i != index )
			if( InfoClients[i].IsEnable() )
				Send(SocketClients[i], data, sizeOfData);
	}
}

CInfoWifi* CWifiServer::GetReferenceOnInfoClientByCID(TCID cid)
{
	for (TIndex i = 0; i < GetNumberClient(); i++)
	{
		if( InfoClients[i].GetCid() == cid )
			return &(InfoClients[i]);
	}

	return NULL;
}

CInfoWifi* CWifiServer::GetReferenceOnInfoClientByIndex(TIndex index)
{
	if( index >= GetNumberClient() )
	{
		cerr<<"Error : CWifiServer::GetReferenceOnInfoClientByIndex : " << index <<" >= "<<GetNumberClient()<<endl;
		return NULL;
	}

	return &(InfoClients[index]);
}
