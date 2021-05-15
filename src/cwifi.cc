#include <math.h>    // log10
#include <stdlib.h>  // rand

#include <iostream>

#include <netlink/netlink.h> // (struct nlmsghdr *)

#include "cwifi.h"

//#include "config.h"

const float ConstanteC=92.45;
const TFrequency Frequency=2.4; // GHz

const int MTU=1640; // Maximum Transmission Unit : 1640 is an experimental value

// distance : meter
int CWifi::Attenuation(TDistance distance)
{
	if( distance == 0 )
		return 0;

	return ConstanteC+20*log10(Frequency)+20*log10(distance/1000);
}

TPower CWifi::BoundedPower(int power)
{
	if( power < TPower_MIN )
		return TPower_MIN;
	if( power > TPower_MAX )
		return TPower_MAX;
	return power;
}

bool CWifi::PacketIsLost(TPower signalLevel)
{
	//don't forget : signalLevel is negative

	int alea = rand() % 53 + 40; // between 40 and 92
	if( alea > -signalLevel )
		return false;

	return true;
}

ssize_t CWifi::SendSignalWithSocket(CSocket* socket, TDescriptor descriptor, TPower* power, const char* buffer, int sizeOfBuffer)
{
//	cout<<"send power : "<<power<<endl;
	int val=socket->Send(descriptor, (char*)power, sizeof(TPower));
	if( val <= 0 )
		return val;

//	std::cout<<"send big data of size : "<<sizeOfBuffer<<std::endl;
	return socket->Send(descriptor, buffer, sizeOfBuffer);
}

ssize_t CWifi::RecvSignalWithSocket(CSocket* socket, TDescriptor descriptor, TPower* power, CDynBuffer* buffer)
{
	int valread;

	// read the power
	valread = socket->Read(descriptor, (char*)power, sizeof(TPower));
	if ( valread <= 0 )
		return valread;

	// read the signal
	// "nlmsg_len" (type "uint32_t") is the first attribut of the "struct nlmsghdr" in "libnl3/netlink/netlink-kernel.h"
	ssize_t sizeRead = socket->ReadEqualSize(descriptor, buffer, 0, sizeof(struct nlmsghdr));
	if( sizeRead == SOCKET_ERROR  )
		return SOCKET_ERROR;

	int sizeTotal=((struct nlmsghdr *)(buffer->GetBuffer()))->nlmsg_len;

	if( sizeTotal > MTU ) // to avoid that a error packet overfulls the memory
		return SOCKET_ERROR;

	return socket->ReadEqualSize(descriptor, buffer, sizeRead, sizeTotal);
}
