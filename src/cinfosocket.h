#ifndef _CINFOSOCKET_H_
#define _CINFOSOCKET_H_

#include <iostream> // ostream

#include "types.h" // TDescriptor

using namespace std;

class CInfoSocket
{
		TDescriptor Descriptor;
		bool Enable;

		void EnableIt();

	public :

		CInfoSocket();

		CInfoSocket(TDescriptor descriptor);

		void SetDescriptor(TDescriptor descriptor);

		TDescriptor GetDescriptor();

		void DisableIt();

		bool IsEnable();

		void Close();

};

#endif
