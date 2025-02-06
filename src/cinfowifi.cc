#include <assert.h> // assert

#include <config.h>
#include "vwifi_config.h" // MAX_SIZE_NAME
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

void CInfoWifi::SetName(string name)
{
	if( name.size() > MAX_SIZE_NAME )
		name.resize(MAX_SIZE_NAME);

	Name=name;
}

string CInfoWifi::GetName() const
{
	return Name;
}

int CInfoWifi::GetSizeName() const
{
	return Name.size();
}

bool CInfoWifi::HasName() const
{
	return ! Name.empty();
}

void CInfoWifi::Display(ostream& os) const
{
	os << Cid << " ";
	if( HasName() )
		os << "("<<Name<<") ";
	CCoordinate::Display(os);
}

ostream& operator<<(ostream& os, const CInfoWifi& infowifi)
{
	infowifi.Display(os) ;
	return os;
}
