#ifndef _CSOCKETCLIENTVHOST_H_
#define _CSOCKETCLIENTVHOST_H_

#include <netinet/ip.h> // struct sockaddr_in
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "csocketclient.h"

class CSocketClientVHOST : public CSocketClient
{
	private :

		struct sockaddr_vm Server;

	public :

		CSocketClientVHOST();

		void Init(TPort port);

		bool Connect() override;

		int GetID() override;
};

#endif
