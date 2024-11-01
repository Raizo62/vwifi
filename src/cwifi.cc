#include <math.h>    // log10
#include <stdlib.h>  // rand

#include <iostream>

#include <netlink/netlink.h> // (struct nlmsghdr *)

#include "hwsim.h" // HWSIM_ATTR_FREQ
#include <netlink/genl/genl.h> // genlmsg_parse

#include "cwifi.h"

//#include "config.h"

const double ConstanteC=92.45;
const TFrequency DEFAULT_FREQUENCY=2412; // Hz

const int MTU=1640; // Maximum Transmission Unit : 1640 is an experimental value

TFrequency CWifi::GetFrequency(struct nlmsghdr* nlh)
{
	/* we get the attributes*/
	struct nlattr *attrs[HWSIM_ATTR_FREQ + 1];
	genlmsg_parse(nlh, 0, attrs, HWSIM_ATTR_FREQ, NULL);

	/* we get frequence */
	if (attrs[HWSIM_ATTR_FREQ])
		return nla_get_u32(attrs[HWSIM_ATTR_FREQ]);
	else
		return DEFAULT_FREQUENCY;
}

// distance : meter
// frequency : Hz
int CWifi::Attenuation(TDistance distance, TFrequency frequency)
{
	if( distance == 0 )
		return 0;

	//     ConstanteC+20*log10(frequency/1000)+20*log10(distance/1000);
	//     ConstanteC+20*(log10(frequency)-log10(1000))+20*(log10(distance)-log10(1000))
	return ConstanteC+20*(log10(frequency)-3)+20*(log10(distance)-3);
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
	int val=socket->Send(descriptor, reinterpret_cast<const char*>(power), sizeof(TPower));
	if( val <= 0 )
		return val;

//	std::cout<<"send big data of size : "<<sizeOfBuffer<<std::endl;
	return socket->Send(descriptor, buffer, sizeOfBuffer);
}

ssize_t CWifi::RecvSignalWithSocket(CSocket* socket, TDescriptor descriptor, TPower* power, CDynBuffer* buffer)
{
	int valread;

	// read the power
	valread = socket->Read(descriptor, reinterpret_cast<char*>(power), sizeof(TPower));
	if ( valread <= 0 )
		return valread;

	// read the signal
	// "nlmsg_len" (type "uint32_t") is the first attribut of the "struct nlmsghdr" in "libnl3/netlink/netlink-kernel.h"
	ssize_t sizeRead = socket->ReadEqualSize(descriptor, buffer, 0, sizeof(struct nlmsghdr));
	if( sizeRead == SOCKET_ERROR  )
		return SOCKET_ERROR;

	int sizeTotal=reinterpret_cast<struct nlmsghdr *>(buffer->GetBuffer())->nlmsg_len;

	if( sizeTotal > MTU ) // to avoid that a error packet overfulls the memory
		return SOCKET_ERROR;

	return socket->ReadEqualSize(descriptor, buffer, sizeRead, sizeTotal);
}
