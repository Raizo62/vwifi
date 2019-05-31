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

#include <regex>

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
			//dellink(h);
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
#ifdef _DEBUG	
			std::cout << inetdevice << std::endl ;
#endif	
		}
	}
}


