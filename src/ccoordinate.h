#ifndef _CCOORDINATE_H_
#define _CCOORDINATE_H_

#include <iostream>

typedef int Value;

using namespace std;

class CCoordinate
{
		Value X;
		Value Y;
		Value Z;

	public :
		CCoordinate();
		CCoordinate(Value x, Value y, Value z);

		void SetX(Value x);
		void SetY(Value y);
		void SetZ(Value z);

		void Set(Value x, Value y);
		void Set(Value x, Value y, Value z);

		void Set(CCoordinate coo);

		Value GetX();
		Value GetY();
		Value GetZ();

		float DistanceWith(Value x, Value y, Value z);
		float DistanceWith(CCoordinate coo);

		void Display(ostream& os) const;
		friend ostream& operator<<(ostream& os, const CCoordinate& coo);

};

#endif
