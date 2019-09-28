#include <math.h>    // log10
#include <stdlib.h>  // rand

#include "config.h"

#include "tpower.h"

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
	//don't forget : signalLevel is negative

	int alea = rand() % 53 + 40; // between 40 and 92
	if( alea > -signalLevel )
		return false;

	return true;
}

