#ifndef _CSOCKETSERVER_H_
#define _CSOCKETSERVER_H_

#include <vector> // vector

#include "csocket.h"
#include "cinfosocket.h"
#include "types.h" // TIndex

class CSocketServer : public CSocket
{
	protected :

		TPort Port;

		std::vector<CInfoSocket> SocketClients;

		TDescriptor Accept(struct sockaddr_in& address);

		TDescriptor GetSocketClient(TIndex index);

	public :

		CSocketServer();

		CSocketServer(TSocket type);

		~CSocketServer();

		void Init(TPort port);

		virtual bool Listen();

		virtual TDescriptor Accept();

		TIndex GetNumberClient();

		bool IsEnable(TIndex index);

		void DisableClient(TIndex index);

		void CloseClient(TIndex index);

		ssize_t Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData);

		ssize_t Read(TDescriptor descriptor, char* data, ssize_t sizeOfData);

		TDescriptor operator[] (TIndex index);
};

#endif
