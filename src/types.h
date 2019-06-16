#ifndef _TYPES_H_
#define _TYPES_H_

typedef int TValue;

typedef int TCID;
const TCID TCID_DISABLE=0;

enum TOrder {
	TORDER_NO, TORDER_LIST, TORDER_CHANGE_COORDINATE
};

typedef int TDescriptor;

typedef unsigned int TIndex;

typedef int TSocket;
// AF_INET : use IP
// AF_VSOCK : use vsock

typedef unsigned int TPort;

#endif
