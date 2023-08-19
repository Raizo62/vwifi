/*
    From : https://android.googlesource.com/device/generic/goldfish/+/refs/heads/master/wifi/mac80211_create_radios/main.cpp
        Licence http://www.apache.org/licenses/LICENSE-2.0
*/

#include <memory>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/genl.h>
#include <netlink/netlink.h>
#include <net/ethernet.h>

#include <climits>
#include <stdio.h>
#include <unistd.h> // getuid

#include "config_hwsim.h"
#include "addinterfaces.h"
#include "config.h" // DEFAULT_MAC_PREFIX

int ParseInt(const char* str, int* result)
{
	return sscanf(str, "%d", result);
}

int help(FILE* dst, const int ret)
{
	fprintf(dst, "%s",
			"Usage:\n"
			"   vwifi-add-interfaces [-h] [-v] n_radios mac_prefix\n"
			"   vwifi-add-interfaces [--help] [--version] n_radios mac_prefix\n"
			"   where\n"
			"       n_radios - int, [1,100], e.g. 2\n"
			"       mac_prefix - xx:xx:xx:xx:xx (if n_radios > 1 then only the first 5 bytes are used\n"
			"                                    then all bytes are used)\n\n"
			"   vwifi-add-interfaces will create n_radios with MAC addresses xx:xx:xx:xx:xx:nn\n"
			"       where xx:xx:xx:xx:xx is the mac_prefix specified\n"
			"       and   nn is incremented (from zero, and only if n_radios > 1)\n");
	return ret;
}

int main(int argc, char* argv[])
{
	int nRadios;
	TByte macPrefix[ETH_ALEN]= {};

	if( argc == 1 )
		return help(stdout,0);

	if( ! ParseAddress(DEFAULT_MAC_PREFIX,macPrefix) )
		return help(stderr, 6);
	if( strlen(DEFAULT_MAC_PREFIX) <= 8 )
	{ // if possible, randomize the 4th byte
		srand(time(NULL));
		macPrefix[3]=rand()%100;
	}

	int arg_idx = 1;
	int arg_used = 1;
	while (arg_idx < argc)
	{
		if( ! strcmp("-v", argv[arg_idx]) || ! strcmp("--version", argv[arg_idx]) )
		{
			fprintf(stdout,"Version : %s\n",VERSION);
			return 0;
		}
		if( ! strcmp("-h", argv[arg_idx]) || ! strcmp("--help", argv[arg_idx]) )
		{
			return help(stdout,0);
		}
		if( argv[arg_idx][0] == '-' )
		{
			fprintf(stderr,"Error : unknown parameter : %s\n",argv[arg_idx]);
			return help(stderr, 1);
		}
		switch ( arg_used )
		{
			case 1 :
				if (!ParseInt(argv[arg_idx], &nRadios))
					return help(stderr, 2);
				if (nRadios < 1)
					return help(stderr, 3);
				if (nRadios > 100)
					return help(stderr, 4);

				arg_used++;
				break;
			case 2 :
				if( strlen(argv[arg_idx]) > 17 )
					return help(stderr, 5);
				if( ! ParseAddress(argv[arg_idx],macPrefix) )
					return help(stderr, 6);

				arg_used++;
				break;
		}

		arg_idx++;
	}

	if( getuid() )
	{
		fprintf(stderr,"Error : This program must be run as root!!\n");
		return 2;
	}

	return ManageRadios(nRadios, macPrefix);
}
