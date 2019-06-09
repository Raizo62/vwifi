#ifndef _CCOORDINATE_H_
#define _CCOORDINATE_H_

#include <iostream>

#include "types.h" // TValue

using namespace std;

class CCoordinate
{
		TValue X;
		TValue Y;
		TValue Z;

	public :
		CCoordinate();
		CCoordinate(TValue x, TValue y, TValue z);

		void SetX(TValue x);
		void SetY(TValue y);
		void SetZ(TValue z);

		void Set(TValue x, TValue y);
		void Set(TValue x, TValue y, TValue z);

		void Set(CCoordinate coo);

		TValue GetX();
		TValue GetY();
		TValue GetZ();

		float DistanceWith(TValue x, TValue y, TValue z);
		float DistanceWith(CCoordinate coo);

		void Display(ostream& os) const;
		friend ostream& operator<<(ostream& os, const CCoordinate& coo);

};

#endif
