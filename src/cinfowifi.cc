#include "cinfowifi.h"

CInfoWifi::CInfoWifi(): CCoordinate()
{
	SetCid(0);
}

CInfoWifi::CInfoWifi(TCID cid, CCoordinate coo) : CCoordinate(coo)
{
	SetCid(cid);
}

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
