#include <iostream> // cout
#include <cstdio> //perror
#include <cstring> // memcpy
#include <assert.h> // assert

#include <arpa/inet.h> // INADDR_ANY
#include <sys/socket.h>
#include <linux/vm_sockets.h> // struct sockaddr_vm

#include "csocketserver.h"

using namespace std;

CSocketServer::CSocketServer(CListInfo<CInfoSocket>* infoSockets) : CSocket()
{
	Init(0);

	if( infoSockets == NULL )
	{
		InfoSockets = new CListInfo<CInfoSocket>;

		ListInfoSelfManaged=true;
	}
	else
	{
		InfoSockets = infoSockets;

		ListInfoSelfManaged=false;
	}
}

CSocketServer::CSocketServer( const CSocketServer & socketServer ) : CSocket(socketServer)
{
	*this=socketServer;
}

CSocketServer& CSocketServer::operator=(const CSocketServer& socketServer)
{
	if( this != &socketServer )
	{
		// protect against invalid self-assignment
		Init(socketServer.GetPort());

		ListInfoSelfManaged=socketServer.ListInfoSelfManaged;

		if( ListInfoSelfManaged )
		{
			if( InfoSockets != NULL )
				delete InfoSockets;
			InfoSockets = new CListInfo<CInfoSocket>(*(socketServer.InfoSockets));
		}
		else
		{
			InfoSockets = socketServer.InfoSockets;
		}
	}

	// by convention, always return *this
	return *this;
}

void CSocketServer::Init(TPort port)
{
	Port=port;
}

TPort CSocketServer::GetPort() const
{
	return Port;
}

CSocketServer::~CSocketServer()
{
	for (auto& infoSocket : *InfoSockets)
		infoSocket.Close();

	delete InfoSockets;

	CSocket::Close();
}

bool CSocketServer::Listen()
{
	return _Listen(Master, Port);
}

TDescriptor CSocketServer::Accept(TCID& cid)
{
	TDescriptor new_socket;

	new_socket = _Accept(Master, cid);
	if( new_socket == SOCKET_ERROR )
		return SOCKET_ERROR;

	//add new socket to array of sockets
	InfoSockets->push_back(CInfoSocket(new_socket));

	return new_socket;
}

TDescriptor CSocketServer::Accept()
{
	TCID cid;

	return Accept(cid);
}

TDescriptor CSocketServer::GetSocketClient(TIndex index) const
{
	assert( index < InfoSockets->size() );

	return (*InfoSockets)[index].GetDescriptor();
}

TDescriptor CSocketServer::operator[] (TIndex index)
{
	return GetSocketClient(index);
}

TIndex CSocketServer::GetNumberClient() const
{
	return InfoSockets->size();
}

bool CSocketServer::IsEnable(TIndex index)
{
	assert( index < GetNumberClient() );

	return (*InfoSockets)[index].IsEnable();
}

void CSocketServer::DisableClient(TIndex index)
{
	assert( index < InfoSockets->size() );

	(*InfoSockets)[index].DisableIt();
}

void CSocketServer::CloseClient(TIndex index)
{
	assert( index < InfoSockets->size() );

	(*InfoSockets)[index].Close();

	InfoSockets->erase (InfoSockets->begin()+index);
}

ssize_t CSocketServer::Send(TDescriptor descriptor, const char* data, ssize_t sizeOfData)
{
	return CSocket::Send(descriptor, data, sizeOfData);
}

ssize_t CSocketServer::SendBigData(TDescriptor descriptor, const char* data, TMinimalSize sizeOfData)
{
	return CSocket::SendBigData(descriptor, data, sizeOfData);
}

ssize_t CSocketServer::Read(TDescriptor descriptor, char* data, ssize_t sizeOfData)
{
	return CSocket::Read(descriptor, data, sizeOfData);
}

ssize_t CSocketServer::ReadBigData(TDescriptor descriptor, CDynBuffer* data)
{
	return CSocket::ReadBigData(descriptor, data);
}
