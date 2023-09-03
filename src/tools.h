#ifndef _TOOLS_H_
#define _TOOLS_H_

extern bool HashUsesPort;

unsigned long hash_ipaddr(struct sockaddr_in* addr);

bool isInt(const char *str);

bool isPositiveInt(const char *str);

bool isIntOrFloat(const char *str);

#endif
