#ifndef _CINFOWIFI_H_
#define _CINFOWIFI_H_

#include <iostream> // ostream

#include "ccoordinate.h"
#include "types.h" // TCID

using namespace std;

class CInfoWifi : public CCoordinate
{
		TCID Cid;

	public :

		void SetCid(TCID cid);
		
		TCID GetCid();

		void Display(ostream& os) const;

		friend ostream& operator<<(ostream& os, const CInfoWifi& infowifi);
};

#endif
