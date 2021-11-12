#include "ccoordinate.h"

#include <math.h> // sqrt

TScale Scale=1;

CCoordinate::CCoordinate(TValue x, TValue y, TValue z)
{
	Set(x, y, z);
}

CCoordinate::CCoordinate()
{
	X=0;
	Y=0;
	Z=0;
}

CCoordinate::CCoordinate(const CCoordinate& coo)
{
	*this=coo;
}

void CCoordinate::SetX(TValue x)
{
	X=x;
}
void CCoordinate::SetY(TValue y)
{
	Y=y;
}

void CCoordinate::SetZ(TValue z)
{
	Z=z;
}

void CCoordinate::Set(TValue x, TValue y)
{
	SetX(x);
	SetY(y);
}

void CCoordinate::Set(TValue x, TValue y, TValue z)
{
	Set(x, y);
	SetZ(z);
}

void CCoordinate::Set(CCoordinate coo)
{
	Set(coo.X,coo.Y,coo.Z);
}

TDistance CCoordinate::DistanceWith(TValue x, TValue y, TValue z)
{
	return Scale*sqrt( (X-x)*(X-x)+(Y-y)*(Y-y)+(Z-z)*(Z-z) );
}

TDistance CCoordinate::DistanceWith(CCoordinate coo)
{
	return DistanceWith(coo.X, coo.Y, coo.Z);
}

void CCoordinate::Display(ostream& os) const
{
	os << X << " " << Y << " " << Z;
}

ostream& operator<<(ostream& os, const CCoordinate& coo)
{
	coo.Display(os) ;
	return os;
}

CCoordinate& CCoordinate::operator=(const CCoordinate& coo)
{
	Set(coo.X,coo.Y,coo.Z);
	return *this;
}
