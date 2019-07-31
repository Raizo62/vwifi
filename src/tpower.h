#ifndef _TPOWER_H_
#define _TPOWER_H_
#include "types.h" // TPower

namespace power
{

const float ConstanteC=92.45;
const float Frequency=2.4; // GHz

// distance : meter
int Attenuation(float distance);

bool PacketIsLost(TPower signalLevel);

}

#endif
