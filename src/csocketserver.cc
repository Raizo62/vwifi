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

CSocketServer::CSocketServer(TSocket type, CListInfo<CInfoSocket>* infoSockets) : CSocket(type)
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
	Init(socketServer.GetPort());

	ListInfoSelfManaged=socketServer.ListInfoSelfManaged;

	if( ListInfoSelfManaged )
	{
		InfoSockets = new CListInfo<CInfoSocket>(*(socketServer.InfoSockets));
	}
	else
	{
		InfoSockets = socketServer.InfoSockets;
	}
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
	//create a master socket
	if( ! Configure() )
	{
		cerr<<"Error : CSocketServer::Listen : Configure"<<endl;
		return false;
	}

	//set master socket to allow multiple connections ,
	//this is just a good habit, it will work without this
	int opt = 1 ; // TRUE
	if( setsockopt(Master, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("CSocketServer::Listen : setsockopt : SO_REUSEADDR");
		return false;
	}

	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	if (setsockopt (Master, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("CSocketServer::Listen : setsockopt : SO_RCVTIMEO\n");
	if (setsockopt (Master, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("CSocketServer::Listen : setsockopt : SO_SNDTIMEO\n");

	switch ( Type )
	{
		case AF_VSOCK :
			{
				//type of socket created
				struct sockaddr_vm address;
				address.svm_family = AF_VSOCK;
				address.svm_reserved1 = 0;
				address.svm_port = Port;
				address.svm_cid = VMADDR_CID_ANY;
				memset(address.svm_zero, 0, sizeof(address.svm_zero));

				//bind the socket
				if (bind(Master, (struct sockaddr*)&address, sizeof(address)) != 0)
				{
					perror("CSocketServer::Listen : bind");
					return false;
				}

				break ;
			}

		case AF_INET :
			{
				//type of socket created
				struct sockaddr_in address;
				address.sin_family = AF_INET;
				address.sin_addr.s_addr = INADDR_ANY;
				address.sin_port = htons( Port );

				//bind the socket
				if (bind(Master, (struct sockaddr *)&address, sizeof(address))<0)
				{
					perror("CSocketServer::Listen : bind");
					return false;
				}

				break;
			}
	}

	cout<<"Listener on port : "<<Port<<endl;
	//try to specify maximum of 3 pending connections for the master socket
	if( listen(Master, 3) < 0 )
	{
		perror("CSocketServer::Listen : listen");
		return false;
	}

	return true;
}

TDescriptor CSocketServer::Accept()
{
	struct sockaddr_in address;

	return Accept(address);
}

TDescriptor CSocketServer::Accept(struct sockaddr_in& address)
{
	TDescriptor new_socket;

	int addrlen = sizeof(address);

	if ((new_socket = accept(Master, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
	{
		perror("CSocketServer::Accept : accept");
		return SOCKET_ERROR;
	}

	//add new socket to array of sockets
	InfoSockets->push_back(CInfoSocket(new_socket));

	return new_socket;
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
	return CSocket::Read(descriptor , data, sizeOfData);
}

ssize_t CSocketServer::ReadBigData(TDescriptor descriptor, CDynBuffer* data)
{
	return CSocket::ReadBigData(descriptor , data);
}
