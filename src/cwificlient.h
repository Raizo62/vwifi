#ifndef _CWIFICLIENT_H_
#define _CWIFICLIENT_H_

#include "ckernelwifi.h"
#include "csocketclient.h"
#include "cwifi.h"

template <typename TypeCSocketClient>
class CWifiClient : public CKernelWifi, public CWifi, public TypeCSocketClient
{
		bool _Connect(int* id) override
		{
			if( ! TypeCSocketClient::_Connect() )
				return false;

			if( ! Scheduler.AddNode(*this) )
				return false;

			*id=TypeCSocketClient::_GetID();

			return true;
		}

		ssize_t _SendSignal(TPower* power, const char* buffer, int sizeOfBuffer) override
			{	return SendSignalWithSocket(this, this->GetDescriptor(), power, buffer, sizeOfBuffer); }

		ssize_t _RecvSignal(TPower* power, CDynBuffer* buffer) override
			{	return RecvSignalWithSocket(this, this->GetDescriptor(), power, buffer); }

		void _Close() override { TypeCSocketClient::Close(); };
};

#endif /* _CWIFICLIENT_H_ */


