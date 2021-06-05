#include <iostream> // cout
#include <cstdio> //perror

#include <sys/ioctl.h> // ioctl
#include <fcntl.h> // open

#include <arpa/inet.h> // INADDR_ANY
#include <unistd.h> // close
#include <assert.h> // assert

#include "csocketclientvtcp.h"
#include "tools.h"

using namespace std;

CSocketClientVTCP::CSocketClientVTCP() : CSocketClient(AF_VSOCK) {}

void CSocketClientVTCP::Init(TPort port)
{
	Server.svm_family = AF_VSOCK;
	Server.svm_reserved1 = 0;
	Server.svm_port = port;
	Server.svm_cid = 2;
}

bool CSocketClientVTCP::Connect()
{
	return ConnectCore((struct sockaddr*) &Server, sizeof(Server));
}

int CSocketClientVTCP::GetID()
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
