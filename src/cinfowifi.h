#ifndef _CINFOWIFI_
#define _CINFOWIFI_

#include <iostream>

using namespace std;

typedef int TypeCID;

class CInfoWifi
{
		TypeCID Cid;

	public :

		void SetCid(TypeCID cid);
		
		TypeCID GetCid();

		void Display(ostream& os) const;

		friend ostream& operator<<(ostream& os, const CInfoWifi& infowifi);
};

#endif
