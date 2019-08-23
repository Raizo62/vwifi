#ifndef _HWSIM_H_
#define _HWSIM_H_

#include <stdint.h>

typedef int8_t s8;
typedef uint8_t u8;
typedef uint16_t u16;

#define __packed	__attribute__((packed))

#define BIT(x)  (1 << (x))

#include "mac80211_hwsim.h"

#define VERSION_NR 1

#endif /* _HWSIM_H_ */


