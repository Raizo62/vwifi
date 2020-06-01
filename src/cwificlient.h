#ifndef _CWIFICLIENT_H_
#define _CWIFICLIENT_H_

#include "ckernelwifi.h"
#include "csocketclient.h"

template <typename TypeCSocketClient>
class CWifiClient : public CKernelWifi, public TypeCSocketClient
{
		bool Connect() {
			if ( ! TypeCSocketClient::Connect() )
				return false;
			return Scheduler.AddNode(*this);
		}

		bool Connect(int* id)
		{
			if( ! Connect() )
				return false;

			*id=TypeCSocketClient::GetID();

			return true;
		}

		ssize_t Send(const char* data, ssize_t sizeOfData){ return TypeCSocketClient::Send(data, sizeOfData); };
		ssize_t SendBigData(const char* data, ssize_t sizeOfData){ return TypeCSocketClient::SendBigData(data, sizeOfData); };

		ssize_t Read(char* data, ssize_t sizeOfData){ return TypeCSocketClient::Read(data, sizeOfData); };
		ssize_t ReadBigData(char* data, ssize_t sizeOfData){ return TypeCSocketClient::ReadBigData(data, sizeOfData); };

		void StopReconnect(bool status){ TypeCSocketClient::StopReconnect(status); };

		void Close(){ TypeCSocketClient::Close(); };
};

#endif /* _CWIFICLIENT_H_ */


