#include <assert.h> // assert

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
	// with the empty constructor : cid=0
	assert( cid==0 || cid >=TCID_GUEST_MIN );
	Cid=cid;
}

TCID CInfoWifi::GetCid() const
{
	return Cid;
}

void CInfoWifi::Display(ostream& os) const
{
	os << Cid << " ";
	CCoordinate::Display(os);
}

ostream& operator<<(ostream& os, const CInfoWifi& infowifi)
{
	infowifi.Display(os) ;
	return os;
}
