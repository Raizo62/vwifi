#ifndef _CSOCKET_H_
#define _CSOCKET_H_

#include <sys/types.h> // ssize_t
#include <sys/socket.h> // AF_INET / AF_VSOCK

#include "config.h" // _USE_VSOCK_BY_DEFAULT_

#include "types.h" // TDescriptor / TSocket

const int SOCKET_ERROR=-1;
const int SOCKET_OK=0;

class CSocket
{
	friend class CWifi;

	protected :

		TDescriptor Master;
		TSocket	Type;

		CSocket();

		CSocket(TSocket type);

		TDescriptor GetDescriptor();

		bool Configure();

		virtual ssize_t Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData);
		virtual ssize_t SendBigData(TDescriptor descriptor, const char* data, TMinimalSize sizeOfData);

		virtual ssize_t Read(TDescriptor descriptor, char* data, ssize_t sizeOfData);
		virtual ssize_t ReadBigData(TDescriptor descriptor, char* data, TMinimalSize sizeOfData);

	public :

		void Close();

		~CSocket();

		operator int();

		TSocket GetType();
};

#endif
