#include <cstddef> // NULL
#include <arpa/inet.h> // struct sockaddr_in & inet_ntoa & ntohs

#include "tools.h"

#include <assert.h> // assert

bool HashUsesPort=false;

unsigned long hash_ipaddr(struct sockaddr_in* addr)
{
	unsigned long res;

	assert( addr != NULL );

	res = (((addr->sin_addr.s_addr >> 24) & 0xff) * 256)	+
			(((addr->sin_addr.s_addr >> 16) & 0xff) * 256)	+
			(((addr->sin_addr.s_addr >>  8) & 0xff)* 256)	+
			(addr->sin_addr.s_addr & 0xff);

	if( HashUsesPort )
		res += addr->sin_port;

	return res;
}

bool isInt(const char *str) {
	while (*str) {
		if( *str < '0' || *str > '9' )
			return false;
		str++;
	}
	return true;
}

bool isIntOrFloat(const char *str) {
	bool seeDot=false;
	bool seeDigit=false;

	if (*str == '-' || *str == '+')
		str++;

	while (*str) {
		if( *str == '.' )
		{
			if( seeDot )
				return false;
			seeDot=true;
		}
		else
		{
			if( *str < '0' || *str > '9' )
				return false;
			seeDigit=true;
		}
		str++;
	}
	return seeDigit;
}
