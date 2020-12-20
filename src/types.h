#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

typedef int8_t s8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;
typedef int32_t s32;
typedef uint32_t u32;

// int
typedef s32 TValue;

// unsigned int
typedef u32 TCID;

enum TOrder {
	TORDER_NO, TORDER_LIST, TORDER_SHOW, TORDER_CHANGE_COORDINATE, TORDER_PACKET_LOSS, TORDER_STATUS, TORDER_DISTANCE_BETWEEN_CID, TORDER_CLOSE_ALL_CLIENT
};

// int
typedef s32 TDescriptor;

// unsigned int
typedef u32 TIndex;

// int
typedef s32 TSocket;
// AF_INET : use IP
// AF_VSOCK : use vsock

// unsigned int
typedef u32 TPort;

// char
typedef s8 TPower; // empirical observed values with int : [-123,20]
	// be careful to constants TPower_MAX and TPower_MIN in cwifi.cc

// float
typedef float TDistance; // in meters

// float
typedef float TFrequency;

// unsigned short
typedef u16 TMinimalSize;

#endif
