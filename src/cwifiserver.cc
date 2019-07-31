#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy
#include <assert.h> // assert

#include <arpa/inet.h> // struct sockaddr_in
#include <sys/socket.h> // AF_VSOCK / AF_INET
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "cwifiserver.h"
#include "tpower.h"

using namespace std;

bool CWifiServer::Listen(TIndex maxClientDeconnected)
{
	MaxClientDeconnected=maxClientDeconnected;

	if( ! CSocketServer::Listen() )
		return false;

	return true;
}

bool CWifiServer::RecoverCoordinateOfInfoWifiDeconnected(TCID cid, CCoordinate& coo)
{
	for (vector<CInfoWifi>::iterator it=InfoWifisDeconnected.begin(); it != InfoWifisDeconnected.end(); ++it)
	{
		if ( it->GetCid() == cid )
		{
			coo=*it;

			InfoWifisDeconnected.erase(it);

			return true;
		}
	}

	return false;
}

bool CWifiServer::RecoverCoordinateOfInfoWifi(TCID cid, CCoordinate& coo)
{
	int index=0;
	for (vector<CInfoWifi>::iterator it = InfoWifis.begin() ; it != InfoWifis.end(); ++it, index++)
	{
		if( IsEnable(index) )
			if( it->GetCid() == cid )
			{
				coo=(*it);

				DisableClient(index);

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
	if( RecoverCoordinateOfInfoWifiDeconnected(cid,coo) || RecoverCoordinateOfInfoWifi(cid, coo) )
		infoWifi.Set(coo);

	infoWifi.SetCid(cid);

	InfoWifis.push_back(infoWifi);

	return new_socket;
}

void CWifiServer::ShowInfoWifi(TIndex index)
{
	assert( index < GetNumberClient() );

	cout<<"{"<<InfoWifis[index]<<"}";
}

void CWifiServer::CloseClient(TIndex index)
{
	assert( index < GetNumberClient() );

	cout<<"Host disconnected : "; ShowInfoWifi(index) ; cout<<endl;

	CSocketServer::CloseClient(index);

	// save the InfoWifi (the coordinate of the cid)
	if( InfoWifisDeconnected.size() >= MaxClientDeconnected )
		InfoWifisDeconnected.erase(InfoWifisDeconnected.begin());
	InfoWifisDeconnected.push_back(InfoWifis[index]);

	InfoWifis.erase(InfoWifis.begin()+index);
}

void CWifiServer::CloseAllClient()
{
	// ( be careful : "TIndex i" is a **unsigned** int : i=0 ; i-1 != -1 but = 65534 )

	TIndex nbre=GetNumberClient(); // because CloseClient changes the value of GetNumberClient()
	for (TIndex i = 0; i < nbre; i++)
		if( IsEnable(0) )
			CloseClient(0); // we can Close the 0 because we use the shift
}

void CWifiServer::SendAllOtherClients(TIndex index,TPower power, const char* data, ssize_t sizeOfData)
{
	CCoordinate coo=InfoWifis[index];
//	cout<<"Forward "<<sizeOfData<<" bytes with "<<power<<" powers"<<endl;

	for (TIndex i = 0; i < GetNumberClient(); i++)
	{
		if( i != index )
			if( IsEnable(i) )
			{
				TPower powerdest=power-power::Attenuation(coo.DistanceWith(InfoWifis[i]));
				//cout<<"distance : "<<coo.DistanceWith(InfoWifis[i])<<" / power : "<<power<<" / Attenuation "<<power::Attenuation(coo.DistanceWith(InfoWifis[i]))<<" / powerdest: "<<powerdest<<endl;
				Send(SocketClients[i].GetDescriptor(), (const char*) &powerdest, sizeof(powerdest));
				Send(SocketClients[i].GetDescriptor(), data, sizeOfData);
			}
	}
}

CInfoWifi* CWifiServer::GetReferenceOnInfoWifiByCID(TCID cid)
{
	for (vector<CInfoWifi>::iterator it = InfoWifis.begin() ; it != InfoWifis.end(); ++it)
	{
		if( it->GetCid() == cid )
			return &(*it);
	}

	return NULL;
}

CInfoWifi* CWifiServer::GetReferenceOnInfoWifiDeconnectedByCID(TCID cid)
{
	for (vector<CInfoWifi>::iterator it = InfoWifisDeconnected.begin() ; it != InfoWifisDeconnected.end(); ++it)
	{
		if( it->GetCid() == cid )
			return &(*it);
	}

	return NULL;
}

CInfoWifi* CWifiServer::GetReferenceOnInfoWifiByIndex(TIndex index)
{
	assert( index < GetNumberClient() );

	return &(InfoWifis[index]);
}
