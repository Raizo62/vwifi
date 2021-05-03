#include <iostream>
#include <string.h> // memcpy

#include "cdynbuffer.h"

const int DEFAULT_SIZE_BUFFER=1024;

CDynBuffer::CDynBuffer()
{
	Buffer = nullptr;
	Size=0;
	Allocate(DEFAULT_SIZE_BUFFER,false);
}

CDynBuffer::~CDynBuffer()
{
	if( Buffer != nullptr )
	{
		delete Buffer;
		Buffer=nullptr;
		Size=0;
	}
}

void CDynBuffer::NeededSize(int size, bool keepValues)
{
	if( size <= Size )
		return;

	Allocate(size,keepValues);
}

char* CDynBuffer::GetBuffer()
{
	return Buffer;
}

void CDynBuffer::Allocate(int size, bool keepValues)
{
	if( ! keepValues && Buffer != nullptr )
		delete Buffer;

	char* newBuffer = new char [size];

	if( keepValues && Buffer != nullptr )
	{
		memcpy(newBuffer,Buffer,Size);
		delete Buffer;
	}

	Buffer = newBuffer;
	Size=size;
}
