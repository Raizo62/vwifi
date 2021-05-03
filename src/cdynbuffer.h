#ifndef _CDYNBUFFER_H_
#define _CDYNBUFFER_H_

class CDynBuffer
{
	private :

		char* Buffer;
		int Size;

		void Allocate(int size, bool keepValues);

	public :

		CDynBuffer();

		~CDynBuffer();

		void NeededSize(int size, bool keepValues);

		char* GetBuffer();

};

#endif
