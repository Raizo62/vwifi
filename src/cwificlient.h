#ifndef _CWIFICLIENT_H_
#define _CWIFICLIENT_H_

#include "ckernelwifi.h"
#include "csocketclient.h"
#include "cwifi.h"

template <typename TypeCSocketClient>
class CWifiClient : public CKernelWifi, public CWifi, public TypeCSocketClient
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

		ssize_t SendSignal(TPower* power, const char* buffer, int sizeOfBuffer)
			{	return SendSignalWithSocket(this, this->GetDescriptor(), power, buffer, sizeOfBuffer); }

		ssize_t RecvSignal(TPower* power, char* buffer, int sizeOfBuffer)
			{	return RecvSignalWithSocket(this, this->GetDescriptor(), power, buffer, sizeOfBuffer); }

		void StopReconnect(bool status){ TypeCSocketClient::StopReconnect(status); };

		void Close(){ TypeCSocketClient::Close(); };
};

#endif /* _CWIFICLIENT_H_ */


