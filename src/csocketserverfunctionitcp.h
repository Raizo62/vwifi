#ifndef _CSOCKETSERVERFUNCTIONITCP_H_
#define _CSOCKETSERVERFUNCTIONITCP_H_

#include "types.h"

class CSocketServerFunctionITCP
{
	private :

		bool Configure(TDescriptor& master);

	protected:

		bool _Listen(TDescriptor& master, TPort port);

		TDescriptor _Accept(TDescriptor master, TCID& cid);

};

#endif
