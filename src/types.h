#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

typedef int8_t s8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;
typedef uint32_t u32;

typedef int TValue;

typedef unsigned int TCID;

enum TOrder {
	TORDER_NO, TORDER_LIST, TORDER_SHOW, TORDER_CHANGE_COORDINATE, TORDER_PACKET_LOSS, TORDER_STATUS, TORDER_DISTANCE_BETWEEN_CID, TORDER_CLOSE_ALL_CLIENT
};

typedef int TDescriptor;

typedef unsigned int TIndex;

typedef int TSocket;
// AF_INET : use IP
// AF_VSOCK : use vsock

typedef unsigned int TPort;

typedef char TPower; // empirical observed values with int : [-123,20]
	// be careful to constants TPower_MAX and TPower_MIN in cwifi.cc

typedef float TDistance; // in meters

typedef float TFrequency;

typedef unsigned short TMinimalSize;

#endif
