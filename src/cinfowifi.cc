#include "cinfowifi.h"

void CInfoWifi::SetCid(TypeCID cid)
{
	Cid=cid;
}

TypeCID CInfoWifi::GetCid()
{
	return Cid;
}

void CInfoWifi::Display(ostream& os) const
{
	os << "[" << Cid << "]";
}

ostream& operator<<(ostream& os, const CInfoWifi& infowifi)
{
	infowifi.Display(os) ;
	return os;
}
