#include <cstddef> // NULL
#include <arpa/inet.h> // struct sockaddr_in & inet_ntoa & ntohs

#include "tools.h"

#include <assert.h> // assert

unsigned long hash_ipaddr(struct sockaddr_in* addr)
{
	unsigned long res;

	assert( addr != NULL );

	res = (((addr->sin_addr.s_addr >> 24) & 0xff) * 256)	+
			(((addr->sin_addr.s_addr >> 16) & 0xff) * 256)	+
			(((addr->sin_addr.s_addr >>  8) & 0xff)* 256)	+
			(addr->sin_addr.s_addr & 0xff);

	return res;
}

bool is_digits(const char *str) {
	while (*str) {
		if( *str < '0' || *str > '9' )
			return false;
		str++;
	}
	return true;
}
