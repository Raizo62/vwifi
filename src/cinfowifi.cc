#include "cinfowifi.h"

void CInfoWifi::SetCid(TCID cid)
{
	Cid=cid;
}

TCID CInfoWifi::GetCid()
{
	return Cid;
}

void CInfoWifi::Display(ostream& os) const
{
	os << "[" << Cid << "]";
	CCoordinate::Display(os);
}

ostream& operator<<(ostream& os, const CInfoWifi& infowifi)
{
	infowifi.Display(os) ;
	return os;
}
