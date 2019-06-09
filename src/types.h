#ifndef _TYPES_H_
#define _TYPES_H_

typedef int TValue;

typedef int TCID;

typedef int TOrder;
const int TORDER_NO=0;
const int TORDER_LIST=1;
const int TORDER_CHANGE_COORDINATE=2;

typedef int Descriptor;

typedef unsigned int Number;

typedef int TypeSocket;
// AF_INET : use IP
// AF_VSOCK : use vsock

typedef unsigned int TPort;

#endif
