#include <iostream>

#include "cdynbuffer.h"

const int DEFAULT_SIZE_BUFFER=1024;

CDynBuffer::CDynBuffer()
{
	Allocate(DEFAULT_SIZE_BUFFER);
}

CDynBuffer::~CDynBuffer()
{
	delete Buffer;
	//Buffer=NULL;
	Size=0;
}

void CDynBuffer::NeededSize(int size)
{
	if( size <= Size )
		return;

	delete Buffer;

	std::cout<<"Allocate from "<<Size<<" to "<<size<<std::endl;
	Allocate(size);
}

char* CDynBuffer::GetBuffer()
{
	return Buffer;
}

void CDynBuffer::Allocate(int size)
{
	Buffer = new char [size];
	Size=size;
}
