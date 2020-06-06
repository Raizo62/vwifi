#include <math.h>    // log10
#include <stdlib.h>  // rand

#include "cwifi.h"

//#include "config.h"

const float ConstanteC=92.45;
const TFrequency Frequency=2.4; // GHz

// distance : meter
int CWifi::Attenuation(float distance)
{
	if( distance == 0 )
		return 0;

	return ConstanteC+20*log10(Frequency)+20*log10(distance/1000);
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

//	cout<<"send big data of size : "<<power<<endl;
	return socket->SendBigData(descriptor, buffer, sizeOfBuffer);
}

ssize_t CWifi::RecvSignalWithSocket(CSocket* socket, TDescriptor descriptor, TPower* power, char* buffer, int sizeOfBuffer)
{
	int valread;

	// read the power
	valread = socket->Read(descriptor, (char*)power, sizeof(TPower));
	if ( valread <= 0 )
		return valread;

	// read the data
	return socket->ReadBigData(descriptor, buffer, sizeOfBuffer);
}
