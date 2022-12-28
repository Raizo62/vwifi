#ifndef _ADDINTERFACE_H_
#define _ADDINTERFACE_H_

#include "types.h"

int ParseAddress(const char* str, TByte addr[ETH_ALEN]);

int ManageRadios(const int nRadios, TByte* macPrefix);

#endif
