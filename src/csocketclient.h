#ifndef _CSOCKETCLIENT_H_
#define _CSOCKETCLIENT_H_

#include "csocket.h"

class CSocketClient : public CSocket
{
		bool IsConnected;

		bool StopTheReconnect;

		void Init();

	protected :

		bool Connect(struct sockaddr* server, size_t size_of_server);

	public :

		CSocketClient();
		virtual ~CSocketClient();

		using CSocket::Send;
		ssize_t Send(const char* data, ssize_t sizeOfData);

		using CSocket::SendBigData;
		ssize_t SendBigData(const char* data, TMinimalSize sizeOfData);

		using CSocket::Read;
		ssize_t Read(char* data, ssize_t sizeOfData);
		
		using CSocket::ReadBigData;
		ssize_t ReadBigData(CDynBuffer* data);

		bool ConnectLoop();

	// virtual :
		virtual bool _Configure() = 0;
		virtual bool _Connect() = 0;
		virtual int _GetID() = 0;
};

#endif
