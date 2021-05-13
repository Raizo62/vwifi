#include <unistd.h> // close
#include <assert.h> // assert

#include "cinfosocket.h"

CInfoSocket::CInfoSocket()
{
	SetDescriptor(-1);
	DisableIt();
}

CInfoSocket::CInfoSocket(TDescriptor descriptor)
{
	SetDescriptor(descriptor);
	EnableIt();
}

void CInfoSocket::SetDescriptor(TDescriptor descriptor)
{
	Descriptor=descriptor;
	EnableIt();
}

TDescriptor CInfoSocket::GetDescriptor() const
{
	return Descriptor;
}

void CInfoSocket::EnableIt()
{
	assert( Descriptor >= 0 );

	Enable=true;
}

void CInfoSocket::DisableIt()
{
	Enable=false;
}

bool CInfoSocket::IsEnable()
{
	return Enable;
}

void CInfoSocket::Close()
{
	if( IsEnable() )
	{
		DisableIt();
		close(Descriptor);
	}
}
