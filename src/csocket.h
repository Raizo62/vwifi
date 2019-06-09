#ifndef _CSOCKET_H_
#define _CSOCKET_H_

#include <sys/types.h> // ssize_t
#include <sys/socket.h> // AF_INET / AF_VSOCK

#include "types.h" // TDescriptor / TSocket

//#define _USE_VSOCK_BY_DEFAULT_

const int SOCKET_ERROR=-1;
const int SOCKET_OK=0;

class CSocket
{
	protected :

		TDescriptor Master;
		TSocket	Type;

		CSocket();

		CSocket(TSocket type);

		bool Configure();

		virtual ssize_t Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData);

		virtual ssize_t Read(TDescriptor descriptor, char* data, ssize_t sizeOfData);

	public :

		void Close();

		~CSocket();

		operator int();
};

#endif
