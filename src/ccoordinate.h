#ifndef _CCOORDINATE_H_
#define _CCOORDINATE_H_

#include <iostream>

#include "types.h" // TValue, TDistance

using namespace std;

extern TScale Scale;

class CCoordinate
{
		TValue X;
		TValue Y;
		TValue Z;

	public :
		CCoordinate();
		CCoordinate(TValue x, TValue y, TValue z);
		CCoordinate(const CCoordinate& coo);

		void SetX(TValue x);
		void SetY(TValue y);
		void SetZ(TValue z);

		void Set(TValue x, TValue y);
		void Set(TValue x, TValue y, TValue z);

		void Set(CCoordinate coo);

		TDistance DistanceWith(TValue x, TValue y, TValue z);
		TDistance DistanceWith(CCoordinate coo);

		void Display(ostream& os) const;
		friend ostream& operator<<(ostream& os, const CCoordinate& coo);

		CCoordinate& operator=(const CCoordinate& coo);
};

#endif
