#ifndef _CSOCKETSERVERFUNCTIONVTCP_H_
#define _CSOCKETSERVERFUNCTIONVTCP_H_

#include "types.h"

class CSocketServerFunctionVTCP
{
	private :

		bool Configure(TDescriptor& master);

	protected:

		bool _Listen(TDescriptor& master, TPort port);

		TDescriptor _Accept(TDescriptor master, TCID& cid);

};

#endif
