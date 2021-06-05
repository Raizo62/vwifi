#ifndef _CSOCKETCLIENTINET_H_
#define _CSOCKETCLIENTINET_H_

#include "csocketclient.h"

#include <netinet/ip.h> // struct sockaddr_in

class CSocketClientINET : public CSocketClient
{
	private :

		struct sockaddr_in Server;

	public :

		CSocketClientINET();

		void Init(const char* IP, TPort port);

		bool Connect() override;

		int GetID() override;
};

#endif
