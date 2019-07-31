#include <cstdio> //perror

#include <iostream> // cout
#include <math.h>   // log10

#include "tpower.h"

using namespace std;
using namespace power;

// distance : meter
int power::Attenuation(float distance)
{
	return ConstanteC+20*log10(Frequency)+20*log10(distance/1000);
}

