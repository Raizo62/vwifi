#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy
#include <assert.h> // assert

#include <arpa/inet.h> // struct sockaddr_in
#include <sys/socket.h> // AF_VSOCK / AF_INET
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "cwifiserver.h"
#include "tools.h"

using namespace std;

CWifiServer::CWifiServer() : CSocketServer ()
{
    InfoWifis = new CListInfo<CInfoWifi>;
    InfoWifisDeconnected = new CListInfo<CInfoWifi>;
}

CWifiServer::CWifiServer(TSocket type) : CSocketServer (type)
{
    InfoWifis = new CListInfo<CInfoWifi>;
    InfoWifisDeconnected = new CListInfo<CInfoWifi>;
}

CWifiServer::~CWifiServer()
{
    delete InfoWifis;
    delete InfoWifisDeconnected;
}

bool CWifiServer::Listen(TIndex maxClientDeconnected)
{
	MaxClientDeconnected=maxClientDeconnected;
	SetPacketLoss(LOST_PACKET_BY_DEFAULT);

	if( ! CSocketServer::Listen() )
		return false;

	return true;
}

bool CWifiServer::RecoverCoordinateOfInfoWifiDeconnected(TCID cid, CCoordinate& coo)
{
	for (auto infoWifiDeconnected = InfoWifisDeconnected->begin(); infoWifiDeconnected != InfoWifisDeconnected->end(); ++infoWifiDeconnected)
	{
		if ( infoWifiDeconnected->GetCid() == cid )
		{
			coo=*infoWifiDeconnected;

			InfoWifisDeconnected->erase(infoWifiDeconnected);

			return true;
		}
	}

	return false;
}

bool CWifiServer::RecoverCoordinateOfInfoWifi(TCID cid, CCoordinate& coo)
{
	int index=0;
	for (auto& infoWifi : *InfoWifis)
	{
		if( IsEnable(index) )
			if( infoWifi.GetCid() == cid )
			{
				coo=infoWifi;

				DisableClient(index);

				return true;
			}
		index++;
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
		cid=hash_ipaddr(&address);

	CInfoWifi infoWifi;

	CCoordinate coo;
	if( RecoverCoordinateOfInfoWifiDeconnected(cid,coo) || RecoverCoordinateOfInfoWifi(cid, coo) )
		infoWifi.Set(coo);

	infoWifi.SetCid(cid);

	InfoWifis->push_back(infoWifi);

	return new_socket;
}

void CWifiServer::ShowInfoWifi(TIndex index)
{
	assert( index < GetNumberClient() );

	cout<<"{"<<(*InfoWifis)[index]<<"}";
}

void CWifiServer::CloseClient(TIndex index)
{
	assert( index < GetNumberClient() );

	CSocketServer::CloseClient(index);

	// save the InfoWifi (the coordinate of the cid)
	AddInfoWifiDeconnected( (*InfoWifis)[index] );

	InfoWifis->erase(InfoWifis->begin()+index);
}

void CWifiServer::CloseAllClient()
{
	// ( be careful : "TIndex i" is a **unsigned** int : i=0 ; i-1 != -1 but = 65534 )

	TIndex nbre=GetNumberClient(); // because CloseClient changes the value of GetNumberClient()
	for (TIndex i = 0; i < nbre; i++)
		CloseClient(0); // we can Close the 0 because we use the shift
}

void CWifiServer::SendAllOtherClients(TIndex index,TPower power, const char* data, ssize_t sizeOfData)
{
	CCoordinate coo=(*InfoWifis)[index];
//	cout<<"Forward "<<sizeOfData<<" bytes with "<<power<<" powers"<<endl;

	for (TIndex i = 0; i < GetNumberClient(); i++)
	{
		if( i != index )
			if( IsEnable(i) )
			{
				TPower signalLevel=BoundedPower(power-Attenuation(coo.DistanceWith((*InfoWifis)[i])));
				if( ! CanLostPackets() || ! PacketIsLost(signalLevel) )
					if( SendSignalWithSocket(this, (*InfoSockets)[i].GetDescriptor(), &signalLevel, data, sizeOfData) < 0 )
						(*InfoSockets)[i].DisableIt();
			}
	}
}

void CWifiServer::SendAllClients(CCoordinate cooSource, TPower power, const char* data, ssize_t sizeOfData)
{
//	cout<<"Forward "<<sizeOfData<<" bytes with "<<power<<" powers"<<endl;

	for (TIndex i = 0; i < GetNumberClient(); i++)
	{
		if( IsEnable(i) )
		{
			TPower signalLevel=BoundedPower(power-Attenuation(cooSource.DistanceWith((*InfoWifis)[i])));
			if( ! CanLostPackets() || ! PacketIsLost(signalLevel) )
				if( SendSignalWithSocket(this, (*InfoSockets)[i].GetDescriptor(), &signalLevel, data, sizeOfData) < 0 )
					(*InfoSockets)[i].DisableIt();
		}
	}
}

void CWifiServer::SendAllClientsWithoutLoss(TPower power, const char* data, ssize_t sizeOfData)
{
	for (TIndex i = 0; i < GetNumberClient(); i++)
		if( IsEnable(i) )
			if( SendSignalWithSocket(this, (*InfoSockets)[i].GetDescriptor(), &power, data, sizeOfData) < 0 )
				(*InfoSockets)[i].DisableIt();
}

CInfoWifi* CWifiServer::GetReferenceOnInfoWifiByCID(TCID cid)
{
	for (auto& infoWifi : *InfoWifis)
	{
		if( infoWifi.GetCid() == cid )
			return &infoWifi;
	}

	return NULL;
}

CInfoWifi* CWifiServer::GetReferenceOnInfoWifiDeconnectedByCID(TCID cid)
{
	for (auto& infoWifiDeconnected : *InfoWifisDeconnected)
	{
		if( infoWifiDeconnected.GetCid() == cid )
			return &infoWifiDeconnected;
	}

	return NULL;
}

CInfoWifi* CWifiServer::GetReferenceOnInfoWifiByIndex(TIndex index)
{
	assert( index < GetNumberClient() );

	return &((*InfoWifis)[index]);
}

void CWifiServer::AddInfoWifiDeconnected(CInfoWifi infoWifi)
{
	if( InfoWifisDeconnected->size() >= MaxClientDeconnected )
		InfoWifisDeconnected->erase(InfoWifisDeconnected->begin());

	InfoWifisDeconnected->push_back(infoWifi);
}

void CWifiServer::SetPacketLoss(bool enable)
{
	PacketLoss=enable;
}

bool CWifiServer::CanLostPackets()
{
	return PacketLoss;
}
