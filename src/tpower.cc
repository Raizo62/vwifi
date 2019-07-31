#include <cstdio> //perror

#include <iostream> // cout
#include <math.h>   // log10

#include "config.h"

#include "tpower.h"

using namespace std;
using namespace power;

// distance : meter
int power::Attenuation(float distance)
{
	if( distance == 0 )
		return 0;

	return ConstanteC+20*log10(Frequency)+20*log10(distance/1000);
}

bool power::PacketIsLost(TPower signalLevel)
{
#ifdef CAN_LOST_PACKET

	//don't forget : signalLevel is negative

	int alea = rand() % 61 + 40; // between 40 and 100
	if( alea > -signalLevel )
		return false;

	return true;

#else

	return false;

#endif

}

