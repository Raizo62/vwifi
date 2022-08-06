#include <iostream> // cout
#include <cstdio> //perror

#include <sys/ioctl.h> // ioctl
#include <fcntl.h> // open

#include <arpa/inet.h> // INADDR_ANY
#include <unistd.h> // close
#include <assert.h> // assert
#include <cstring> // memset

#include "csocketclientvtcp.h"
#include "tools.h"

using namespace std;

CSocketClientVTCP::CSocketClientVTCP() : CSocketClient()
{
}

void CSocketClientVTCP::Init(TPort port)
{
	memset(&Server, 0, sizeof(Server));
	Server.svm_family = AF_VSOCK;
	Server.svm_port = port;
	Server.svm_cid = 2;
}

bool CSocketClientVTCP::_Configure()
{
	Master = socket(AF_VSOCK , SOCK_STREAM , 0);

	if( Master == SOCKET_ERROR )
	{
		perror("CSocketClientVTCP::Configure : socket");
		return false;
	}

	return true;
}

bool CSocketClientVTCP::_Connect()
{
	return Connect((struct sockaddr*) &Server, sizeof(Server));
}

int CSocketClientVTCP::_GetID()
{
	int cid;

	int ioctl_fd = open("/dev/vsock", 0);
	if (ioctl_fd < 0)
	{
		perror("Error : CSocketClientVHOST::GetID : open /dev/vsock :");
		return -1;
	}
	if( ioctl(ioctl_fd, IOCTL_VM_SOCKETS_GET_LOCAL_CID, &cid) < 0 )
	{
		perror("Error : CSocketClientVHOST::GetID : ioctl : Cannot get local CID :");
		close(ioctl_fd);
		return -1;
	}

	close(ioctl_fd);
	return cid;
}
