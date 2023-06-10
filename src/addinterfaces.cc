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

#include "addinterfaces.h"
#include "config_hwsim.h"

const char* nlErrStr(const int e)
{
	return (e < 0) ? nl_geterror(e) : "";
}

#define RETURN(R) return (R);

#define RETURN_ERROR(C, R) \
    do { \
        fprintf(stderr,"%s:%d '%s' failed\n", __func__, __LINE__, C); \
        return (R); \
    } while (false);

#define RETURN_NL_ERROR(C, NLR, R) \
    do { \
        fprintf(stderr,"%s:%d '%s' failed with '%s'\n", __func__, __LINE__, C, nlErrStr((NLR))); \
        return (R); \
    } while (false);

struct nl_sock_deleter
{
	void operator()(struct nl_sock* x) const
	{
		nl_socket_free(x);
	}
};

struct nl_msg_deleter
{
	void operator()(struct nl_msg* x) const
	{
		nlmsg_free(x);
	}
};

int ParseAddress(const char* str, TByte addr[ETH_ALEN])
{
	return sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
				  &addr[0], &addr[1], &addr[2],&addr[3], &addr[4], &addr[5]);
}

std::unique_ptr<struct nl_msg, nl_msg_deleter> CreateNlMessage(
	const int family,
	const int cmd)
{
	std::unique_ptr<struct nl_msg, nl_msg_deleter> msg(nlmsg_alloc());
	if (!msg)
	{
		RETURN_ERROR("nlmsg_alloc", nullptr);
	}
	void* user = genlmsg_put(msg.get(), NL_AUTO_PORT, NL_AUTO_SEQ, family, 0,
							 NLM_F_REQUEST, cmd, VERSION_NR);
	if (!user)
	{
		RETURN_ERROR("genlmsg_put", nullptr);
	}
	RETURN(msg);
}

std::unique_ptr<struct nl_msg, nl_msg_deleter>
BuildCreateRadioMessage(const int family, const TByte mac[ETH_ALEN])
{
	std::unique_ptr<struct nl_msg, nl_msg_deleter> msg =
		CreateNlMessage(family, HWSIM_CMD_NEW_RADIO);
	if (!msg)
	{
		RETURN(nullptr);
	}
	int ret;
	ret = nla_put(msg.get(), HWSIM_ATTR_PERM_ADDR, ETH_ALEN, mac);
	if (ret)
	{
		RETURN_NL_ERROR("nla_put(HWSIM_ATTR_PERM_ADDR)", ret, nullptr);
	}
	ret = nla_put_flag(msg.get(), HWSIM_ATTR_SUPPORT_P2P_DEVICE);
	if (ret)
	{
		RETURN_NL_ERROR("nla_put(HWSIM_ATTR_SUPPORT_P2P_DEVICE)", ret, nullptr);
	}
	RETURN(msg);
}

int CreateRadios(struct nl_sock* socket, const int netlinkFamily,
				 const int nRadios, TByte* mac)
{
	for (int idx = 0; idx < nRadios; ++idx)
	{
		if( nRadios != 1 ) // if i set only 1 interface, the mac address is not modified : the user can choose all the mac address
			mac[5] = idx;
		std::unique_ptr<struct nl_msg, nl_msg_deleter> msg =
			BuildCreateRadioMessage(netlinkFamily, mac);
		if (msg)
		{
			int ret = nl_send_auto(socket, msg.get());
			if (ret < 0)
			{
				RETURN_NL_ERROR("nl_send_auto", ret, 1);
			}
		}
		else
		{
			RETURN(1);
		}
	}
	RETURN(0);
}

int ManageRadios(const int nRadios, TByte* macPrefix)
{
	std::unique_ptr<struct nl_sock, nl_sock_deleter> socket(nl_socket_alloc());
	if (!socket)
	{
		RETURN_ERROR("nl_socket_alloc", 1);
	}
	int ret;
	ret = genl_connect(socket.get());
	if (ret)
	{
		RETURN_NL_ERROR("genl_connect", ret, 1);
	}
	const int netlinkFamily = genl_ctrl_resolve(socket.get(), KERNEL_HWSIM_FAMILY_NAME);
	if (netlinkFamily < 0)
	{
		RETURN_NL_ERROR("genl_ctrl_resolve", ret, 1);
	}
	ret = CreateRadios(socket.get(), netlinkFamily, nRadios, macPrefix);
	if (ret)
	{
		RETURN(ret);
	}
	RETURN(0);
}
