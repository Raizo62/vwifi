#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy
#include <assert.h> // assert

#include <arpa/inet.h> // struct sockaddr_in
#include <sys/socket.h> // AF_VSOCK / AF_INET
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "cwifiserver.h"
#include "tools.h"
#include "config.h" // LOST_PACKET_BY_DEFAULT

using namespace std;

CWifiServer::CWifiServer() : CSocketServer ()
{
	DefaultValues();

	InfoWifis = new CListInfo<CInfoWifi>;
	InfoWifisDeconnected = new CListInfo<CInfoWifi>;
}

CWifiServer::CWifiServer(CListInfo<CInfoSocket>* infoSockets, CListInfo<CInfoWifi>* infoWifis, CListInfo<CInfoWifi>* infoWifisDeconnected) : CSocketServer (infoSockets)
{
	DefaultValues();

	InfoWifis = infoWifis;
	InfoWifisDeconnected = infoWifisDeconnected;
}

CWifiServer::CWifiServer( const CWifiServer & wifiServer ) : CSocketServer(wifiServer), CWifi(wifiServer)
{
	*this=wifiServer;
}

CWifiServer::~CWifiServer()
{
	if( ListInfoSelfManaged )
	{
		delete InfoWifis;
		delete InfoWifisDeconnected;
	}
}

CWifiServer& CWifiServer::operator=(const CWifiServer& wifiServer)
{
	if( this != &wifiServer )
	{
		// protect against invalid self-assignment
		MaxClientDeconnected=wifiServer.MaxClientDeconnected;
		SetPacketLoss(wifiServer.CanLostPackets());

		if( ListInfoSelfManaged )
		{
			if( InfoWifis != NULL )
			{
				delete InfoWifis;
				delete InfoWifisDeconnected;
			}
			InfoWifis = new CListInfo<CInfoWifi>(*(wifiServer.InfoWifis));
			InfoWifisDeconnected = new CListInfo<CInfoWifi>(*(wifiServer.InfoWifisDeconnected));
		}
		else
		{
			InfoWifis = wifiServer.InfoWifis;
			InfoWifisDeconnected = wifiServer.InfoWifisDeconnected;
		}
	}

	// by convention, always return *this
	return *this;
}

void CWifiServer::DefaultValues()
{
	MaxClientDeconnected=0;
	SetPacketLoss(LOST_PACKET_BY_DEFAULT);
}

bool CWifiServer::Listen(TIndex maxClientDeconnected)
{
	MaxClientDeconnected=maxClientDeconnected;
	SetPacketLoss(LOST_PACKET_BY_DEFAULT);

	if( ! _Listen(Master, Port) )
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
	TCID cid;

	TDescriptor new_socket = CSocketServer::Accept(cid);
	if( new_socket ==  SOCKET_ERROR )
		return SOCKET_ERROR;

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

	cout<<(*InfoWifis)[index];
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

ssize_t CWifiServer::SendSignal(TDescriptor descriptor, TPower* power, const char* buffer, int sizeOfBuffer)
{
	return SendSignalWithSocket(this, descriptor, power, buffer, sizeOfBuffer);
}

ssize_t CWifiServer::RecvSignal(TDescriptor descriptor, TPower* power, CDynBuffer* buffer)
{
	return RecvSignalWithSocket(this, descriptor, power, buffer);
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
					if( SendSignal((*InfoSockets)[i].GetDescriptor(), &signalLevel, data, sizeOfData) < 0 )
						(*InfoSockets)[i].DisableIt();
			}
	}
}

void CWifiServer::SendAllOtherClientsWithoutLoss(TIndex index, TPower power, const char* data, ssize_t sizeOfData)
{
	for (TIndex i = 0; i < GetNumberClient(); i++)
	{
		if( i != index )
			if( IsEnable(i) )
				if( SendSignal((*InfoSockets)[i].GetDescriptor(), &power, data, sizeOfData) < 0 )
					(*InfoSockets)[i].DisableIt();
	}
}

void CWifiServer::SendAllClientsWithoutLoss(TPower power, const char* data, ssize_t sizeOfData)
{
	for (TIndex i = 0; i < GetNumberClient(); i++)
		if( IsEnable(i) )
			if( SendSignal((*InfoSockets)[i].GetDescriptor(), &power, data, sizeOfData) < 0 )
				(*InfoSockets)[i].DisableIt();
}

CInfoWifi* CWifiServer::GetReferenceOnInfoWifiByCID(TCID cid) const
{
	for (auto& infoWifi : *InfoWifis)
	{
		if( infoWifi.GetCid() == cid )
			return &infoWifi;
	}

	return NULL;
}

CInfoWifi* CWifiServer::GetReferenceOnInfoWifiDeconnectedByCID(TCID cid) const
{
	for (auto& infoWifiDeconnected : *InfoWifisDeconnected)
	{
		if( infoWifiDeconnected.GetCid() == cid )
			return &infoWifiDeconnected;
	}

	return NULL;
}

CInfoWifi* CWifiServer::GetReferenceOnInfoWifiByIndex(TIndex index) const
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

bool CWifiServer::CanLostPackets() const
{
	return PacketLoss;
}
