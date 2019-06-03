#include "cmonwirelessdevice.h"
#include "cwirelessdevice.h"

#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include <linux/if_arp.h>

#include <stdexcept>

#include <thread>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/route/link.h>

#include <linux/nl80211.h>

/* allow a call of non static function from static function */
monitorinet::CallFromStaticFunc * MonitorWirelessDevice::forward = nullptr ;



void MonitorWirelessDevice::setNewInetCallback(CallbackFunction cb){

	_newinet_cb = cb ;
}

void MonitorWirelessDevice::setDelInetCallback(CallbackFunction cb){

	_delinet_cb = cb ;
}

void MonitorWirelessDevice::setInitInetCallback(CallbackFunction cb){

	_initinet_cb = cb ;
}



MonitorWirelessDevice::MonitorWirelessDevice(){


	struct sockaddr_nl addr;

	/*  create a netlink route socket */
	if(( _inetsock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
		throw std::runtime_error("couldn't create a AF_NETLINK socket"); 

	/* joins the multicast groups for link notifications */
	std::memset(&addr, 0, sizeof addr);
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;
	
	if (bind(_inetsock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		throw std::runtime_error("couldn't bind to AF_NETLINK socket");


	/* allows calls from  static callback to non static member function */ 
	forward = new monitorinet::CallFromStaticFunc(this);



}

MonitorWirelessDevice::~MonitorWirelessDevice(){

#ifdef _DEBUG
	std::cout << __func__ << std::endl ;
#endif
	if (started())
		stop();

	while(!outside_loop());
	
	clean();
	
}



void MonitorWirelessDevice::clean(){

	close(_inetsock);
}

int MonitorWirelessDevice::main_loop() {

	fd_set rfds;
	struct timeval tv;
	int retval ;


	_outsideloopmutex.lock() ;
		_outsideloop = false ;
	_outsideloopmutex.unlock();


	while (started()) {
		
		FD_ZERO(&rfds);
		FD_CLR(_inetsock, &rfds);
		FD_SET(_inetsock, &rfds);

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
		if (retval == -1){
			perror("select in MonitorWirelessDevice main_loop function");
			stop();
		}
		else 
			if (retval){

#ifdef _DEBUG
				std::cout << __func__ <<  "process event received from AF_NETLINK socket" << std::endl ;
#endif
				recv_inet_event();
			}
	}


	_outsideloopmutex.lock() ;
		_outsideloop = true ;
	_outsideloopmutex.unlock();



	return 0;
}


bool MonitorWirelessDevice::outside_loop(){

	bool outsideorno = false ;
	_outsideloopmutex.lock();
		if (_outsideloop) outsideorno = true ;
	_outsideloopmutex.unlock();

	return outsideorno ;
}

void MonitorWirelessDevice::start() {

	if(started())
		return ;

	if (!outside_loop())
		return ;

	_startedmutex.lock() ;
		_started = true ;
	_startedmutex.unlock();

	std::thread mainloop(&MonitorWirelessDevice::main_loop,this);
	
	mainloop.detach();
}

void MonitorWirelessDevice::stop() {

	_startedmutex.lock();
		_started  = false ;
	_startedmutex.unlock();
}

bool MonitorWirelessDevice::started() {

	bool startorno = false ;
	_startedmutex.lock();
		if (_started) startorno = true ;
	_startedmutex.unlock();

	return startorno ;
}



void MonitorWirelessDevice::recv_inet_event()
{
	int len;
	char buf[IFLIST_REPLY_BUFFER];
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl snl;
	struct msghdr msg = { (void *)&snl, sizeof(snl), &iov, 1, NULL, 0, 0 };
	struct nlmsghdr *nlmsgheader;

	/* read the waiting message */
	len = recvmsg(_inetsock, &msg, 0);
	if (len < 0)
		perror("read_netlink");
	for (nlmsgheader = (struct nlmsghdr *)buf; NLMSG_OK(nlmsgheader, (unsigned int)len); nlmsgheader = NLMSG_NEXT(nlmsgheader, len)) {

		switch (nlmsgheader->nlmsg_type) {
		case NLMSG_DONE:
			//nlmsg_end = 1;
			break;
		case NLMSG_ERROR:
			perror("read_netlink");
			break;
		case RTM_NEWLINK:
#ifdef _DEBUG
			std::cout << "Network interface added" << std::endl ;
#endif
			new_net_interface(nlmsgheader);
			break;
		case RTM_DELLINK:
#ifdef _DEBUG
			std::cout << "Network interface deleted" << std::endl ;
#endif
			del_net_interface(nlmsgheader);

			break;
		default:
			break;
		}
	}
}



void MonitorWirelessDevice::new_net_interface(struct nlmsghdr *h)
{
	int len;
	struct rtattr *tb[IFLA_MAX + 1];
	char *name;
	struct rtattr *rta;
	struct ifinfomsg *ifi;
	struct ether_addr macaddr ;
	ifi = (struct ifinfomsg *) NLMSG_DATA(h);

	/*if (!(ifi->ifi_flags & IFLA_ADDRESS))
	{
		std::cout << "! IFLA_ADDRESS" << std::endl ; 
		return;
	}*/
	
	
	/* retrieve all attributes */
	memset(tb, 0, sizeof(tb));
	rta = IFLA_RTA(ifi);
	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= IFLA_MAX)
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}

	/* require name field to be set */
	if (tb[IFLA_IFNAME]) {
		name = (char *)RTA_DATA(tb[IFLA_IFNAME]);
	} else {
		std::cout << "do not find interface name" << std::endl ;
		return;
	}

#ifdef _DEBUG	
	if (ifi->ifi_family == AF_UNSPEC)
		std::cout << "family:  AF_UNSPEC" << std::endl;

	if (ifi->ifi_family == AF_INET6)
		std::cout << "family:  AF_INET6" << std::endl ;
#endif


	/* require address field to be set */
	if (tb[IFLA_ADDRESS]) {
		
		std::memcpy(&macaddr, RTA_DATA(tb[IFLA_ADDRESS]), ETH_ALEN);
			

		std::string inet_name(name);
	
		WirelessDevice inetdevice (inet_name,ifi->ifi_index,ifi->ifi_type,macaddr);
	
		if(inetdevice.checkif_wireless_device()){

			_newinet_cb(inetdevice);
		}
	}
}



void MonitorWirelessDevice::del_net_interface(struct nlmsghdr *h)
{
	int len;
	struct rtattr *tb[IFLA_MAX + 1];
	char *name;
	struct rtattr *rta;
	struct ifinfomsg *ifi;
	struct ether_addr macaddr ;
	ifi = (struct ifinfomsg *) NLMSG_DATA(h);

	/*if (!(ifi->ifi_flags & IFLA_ADDRESS))
	{
		std::cout << "! IFLA_ADDRESS" << std::endl ; 
		return;
	}*/
	
	
	/* retrieve all attributes */
	memset(tb, 0, sizeof(tb));
	rta = IFLA_RTA(ifi);
	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
	while (RTA_OK(rta, len)) {
		if (rta->rta_type <= IFLA_MAX)
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta, len);
	}

	/* require name field to be set */
	if (tb[IFLA_IFNAME]) {
		name = (char *)RTA_DATA(tb[IFLA_IFNAME]);
	} else {
		std::cout << "do not find interface name" << std::endl ;
		return;
	}

#ifdef _DEBUG	
	if (ifi->ifi_family == AF_UNSPEC)
		std::cout << "family:  AF_UNSPEC" << std::endl;

	if (ifi->ifi_family == AF_INET6)
		std::cout << "family:  AF_INET6" << std::endl ;
#endif


	/* require address field to be set */
	if (tb[IFLA_ADDRESS]) {
		
		std::memcpy(&macaddr, RTA_DATA(tb[IFLA_ADDRESS]), ETH_ALEN);
			

		std::string inet_name(name);
	
		WirelessDevice inetdevice (inet_name,ifi->ifi_index,ifi->ifi_type,macaddr);
	
		if(inetdevice.checkif_wireless_device()){

			_delinet_cb(inetdevice);
		}
	}
}





int MonitorWirelessDevice::get_winterface_infos()
{

	/* init netlink socket with nl80211 module */
	wifi.nls = nl_socket_alloc();

	if (!wifi.nls) {
        
		std::cerr << "Failed to allocate netlink socket." << std::endl ; 
		return -ENOMEM;
	}


	nl_socket_set_buffer_size(wifi.nls, 8192, 8192);

	if (genl_connect(wifi.nls)) {
		
		std::cerr << "Failed to connect to generic netlink" << std::endl ;
		nl_socket_free(wifi.nls);
		return -ENOLINK;
	}

	wifi.nl80211_id = genl_ctrl_resolve(wifi.nls, "nl80211");
	if (wifi.nl80211_id < 0) {
        
		std::cerr << "nl80211 not found." << std::endl ;
	       	nl_socket_free(wifi.nls);
		return -ENOENT;
	}


	/* allocate a msg to send to a module */
	struct nl_msg *msg = nlmsg_alloc();

	if (!msg) {
        
		std::cerr << "Failed to allocate netlink message." << std::endl ;
		return -ENOMEM;
	}

	/*set a callback that receive messages from a module */
	struct nl_cb *cb = nl_cb_alloc(NL_CB_DEFAULT);

	if (!cb) {
        
		std::cerr << "Failed to allocate netlink callback." << std::endl ;
		nlmsg_free(msg);
	      	return -ENOMEM;
	}

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM,recv_winterface_infos_cb, NULL);


	/* send get inerface  command to deriver */
	genlmsg_put(msg, 0, 0, wifi.nl80211_id, 0,NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);

	if (nl_send_auto(wifi.nls, msg) < 0)
	{
		nlmsg_free(msg);
		return -1 ;
	}

	nl_complete_msg(wifi.nls,msg);
	
	if (nl_send(wifi.nls, msg) < 0)
	{
		nlmsg_free(msg);
		return -1;
	}

	/* set a finish callback */
	int err = 1;
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, handle_iee80211_com_finish_cb, &err);

	nl_recvmsgs(wifi.nls, cb);

	/*cleanup*/
	nlmsg_free(msg);
	nl_cb_put(cb);
	nl_socket_free(wifi.nls);

	return 0 ;
	
}


int MonitorWirelessDevice::recv_winterface_infos_cb(struct nl_msg *msg, void *arg){

	forward->recv_winterface_infos(msg,arg);
	return 0 ;
}


int MonitorWirelessDevice::recv_winterface_infos(struct nl_msg *msg, void *arg){


#ifdef _DEBUG
	std::cout << __func__ << std::endl ;
#endif

	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = (struct genlmsghdr *) nlmsg_data(nlmsg_hdr(msg));

     	char *ifname;
	int ifindex;
	int iftype;
	struct ether_addr macaddr ;

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),genlmsg_attrlen(gnlh, 0), NULL);
    
	if (tb_msg[NL80211_ATTR_IFNAME])
        
		ifname = nla_get_string(tb_msg[NL80211_ATTR_IFNAME]) ;

	else
		return NL_SKIP;

	if (tb_msg[NL80211_ATTR_IFINDEX])
		
		ifindex = nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]);
	else
		return NL_SKIP;

	if (tb_msg[NL80211_ATTR_MAC])
	
		std::memcpy(&macaddr, nla_data(tb_msg[NL80211_ATTR_MAC]), ETH_ALEN);
	else
		return NL_SKIP;

	
	if (tb_msg[NL80211_ATTR_IFTYPE])
	
		iftype = nla_get_u32(tb_msg[NL80211_ATTR_IFTYPE]);
	else
		return NL_SKIP;


	std::string inet_name(ifname);
	
	WirelessDevice inetdevice (inet_name,ifindex,iftype,macaddr);

	_initinet_cb(inetdevice);	

	return NL_SKIP;
}



int MonitorWirelessDevice::handle_iee80211_com_finish_cb(struct nl_msg *msg, void *arg){

	forward->handle_iee80211_com_finish(msg,arg);
	return 0 ;

}

int MonitorWirelessDevice::handle_iee80211_com_finish(struct nl_msg *msg, void *arg){

#ifdef _DEBUG
	std::cout << __func__ << std::endl ;
#endif

	int *ret = (int*) arg;
    	*ret = 0;
    	return NL_SKIP;
}


