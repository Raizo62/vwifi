#include "ccoordinate.h"

CCoordinate::CCoordinate(Value x, Value y, Value z)
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

void CCoordinate::SetX(Value x)
{
	X=x;
}
void CCoordinate::SetY(Value y)
{
	Y=y;
}

void CCoordinate::SetZ(Value z)
{
	Z=z;
}

Value CCoordinate::GetX()
{
	return X;
}

Value CCoordinate::GetY()
{
	return Y;
}

Value CCoordinate::GetZ()
{
	return Z;
}

void CCoordinate::Set(Value x, Value y)
{
	SetX(x);
	SetY(y);
}

void CCoordinate::Set(Value x, Value y, Value z)
{
	Set(x, y);
	SetZ(z);
}

void CCoordinate::Set(CCoordinate coo)
{
	*this=coo;
}

float CCoordinate::DistanceWith(Value x, Value y, Value z)
{
	return (X-x)*(X-x)+(Y-y)*(Y-y)+(Z-z)*(Z-z);
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
