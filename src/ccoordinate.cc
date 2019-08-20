#include "ccoordinate.h"

#include <math.h> // sqrt

CCoordinate::CCoordinate(TValue x, TValue y, TValue z)
{
	Set(x, y);
	SetZ(z);
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

TValue CCoordinate::GetX()
{
	return X;
}

TValue CCoordinate::GetY()
{
	return Y;
}

TValue CCoordinate::GetZ()
{
	return Z;
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
	*this=coo;
}

float CCoordinate::DistanceWith(TValue x, TValue y, TValue z)
{
	return sqrt( (X-x)*(X-x)+(Y-y)*(Y-y)+(Z-z)*(Z-z) );
}

float CCoordinate::DistanceWith(CCoordinate coo)
{
	return DistanceWith(coo.X, coo.Y, coo.Z);
}

void CCoordinate::Display(ostream& os) const
{
	os << "(" << X << "," << Y << "," << Z << ")";
}

ostream& operator<<(ostream& os, const CCoordinate& coo)
{
	coo.Display(os) ;
	return os;
}
