#ifndef _CSOCKET_H_
#define _CSOCKET_H_

#include <sys/types.h> // ssize_t
#include <sys/socket.h> // AF_INET / AF_VSOCK

#include "config.h" // _USE_VSOCK_BY_DEFAULT_

#include "types.h" // TDescriptor / TSocket

#include "cdynbuffer.h" // CDynBuffer

const int SOCKET_ERROR=-1;

class CSocket
{
	friend class CWifi;

	protected :

		TDescriptor Master;
		TSocket	Type;

		CSocket();

		explicit CSocket(TSocket type);

		CSocket( const CSocket & socket );

		TDescriptor GetDescriptor() const;

		bool Configure();

		virtual ssize_t Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData);
		virtual ssize_t SendBigData(TDescriptor descriptor, const char* data, TMinimalSize sizeOfData);

		virtual ssize_t Read(TDescriptor descriptor, char* data, ssize_t sizeOfData);
		virtual ssize_t ReadBigData(TDescriptor descriptor, CDynBuffer* data);

	private :
		ssize_t ReadEqualSize(TDescriptor descriptor, CDynBuffer* data, ssize_t byteAlreadyRead, ssize_t sizeToRead);

	public :

		void Close();

		~CSocket();

		operator int();
};

#endif
