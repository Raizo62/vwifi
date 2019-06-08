#ifndef _CSOCKET_
#define _CSOCKET_

#include <sys/types.h> // ssize_t
#include <sys/socket.h> // AF_INET / AF_VSOCK

//#define _USE_VSOCK_BY_DEFAULT_

const int SOCKET_ERROR=-1;
const int SOCKET_OK=0;

typedef int Descriptor;

typedef int TypeSocket;
// AF_INET : use IP
// AF_VSOCK : use vsock

class CSocket
{
	protected :

		Descriptor Master;
		TypeSocket	Type;

		CSocket();

		CSocket(TypeSocket type);

		bool Configure();

		virtual ssize_t Send(Descriptor descriptor, const char* data, ssize_t sizeOfData);

		virtual ssize_t Read(Descriptor descriptor, char* data, ssize_t sizeOfData);

	public :

		void Close();

		~CSocket();

		operator int();
};

#endif
