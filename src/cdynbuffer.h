#ifndef _CDYNBUFFER_H_
#define _CDYNBUFFER_H_

class CDynBuffer
{
	private :

		char* Buffer;
		int Size;

		void Allocate(int size);

	public :

		CDynBuffer();

		~CDynBuffer();

		void NeededSize(int size);

		char* GetBuffer();

};

#endif
