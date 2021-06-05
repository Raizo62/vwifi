#ifndef _CSOCKETCLIENTVTCP_H_
#define _CSOCKETCLIENTVTCP_H_

#include <netinet/ip.h> // struct sockaddr_in
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "csocketclient.h"

class CSocketClientVTCP : public CSocketClient
{
	private :

		struct sockaddr_vm Server;

	public :

		CSocketClientVTCP();

		void Init(TPort port);

		bool Connect() override;

		int GetID() override;
};

#endif
