#ifndef _CSOCKETCLIENTITCP_H_
#define _CSOCKETCLIENTITCP_H_

#include "csocketclient.h"

#include <netinet/ip.h> // struct sockaddr_in

class CSocketClientITCP : public CSocketClient
{
	private :

		struct sockaddr_in Server;

	public :

		CSocketClientITCP();

		void Init(const char* IP, TPort port);

		bool _Configure() override;

		bool _Connect() override;

		int _GetID() override;
};

#endif
