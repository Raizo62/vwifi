#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/route/link.h>

#include <linux/if_arp.h>
#include <linux/nl80211.h>

#include "ieee80211.h"
#include "cvwifiguest.h"

#include <iostream>
#include <thread>
#include <cstring>

#include "config.h"
#include "cmonwirelessdevice.h"


/* allow calling non static function from static function */
vwifiguest::CallFromStaticFunc * VWifiGuest::forward = nullptr ;



void VWifiGuest::set_all_rates_invalid(struct hwsim_tx_rate *tx_rate)
{
	/* Set up all unused rates to be -1 */
	for (int i = 0; i < IEEE80211_MAX_RATES_PER_TX; i++) {
		tx_rate[i].idx = -1;
		tx_rate[i].count = 0;
	}
}


int VWifiGuest::send_tx_info_frame_nl(struct ether_addr *src, unsigned int flags, int signal, struct hwsim_tx_rate *tx_attempts, unsigned long cookie)
{
	struct nl_msg *msg = nullptr;
	int rc;

	msg = nlmsg_alloc();

	if (!msg) {
		
		std::cerr << "Error allocating new message MSG !" << std::endl ;
		nlmsg_free(msg);
		return 0;
	}
	
	if (m_family_id < 0){
		
#ifdef _DEBUG
		std::cout << __func__ <<  "m_family_id < 0" << std::endl ;
#endif
		nlmsg_free(msg);
		return 0;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, m_family_id,
			0, NLM_F_REQUEST, HWSIM_CMD_TX_INFO_FRAME, VERSION_NR);

	/* i have to ack the src the driver expects
	 * so there are no mac address modifications here
	 */
	rc = nla_put(msg, HWSIM_ATTR_ADDR_TRANSMITTER, sizeof(struct ether_addr), src);
	rc = nla_put_u32(msg, HWSIM_ATTR_FLAGS, flags);
	rc = nla_put_u32(msg, HWSIM_ATTR_SIGNAL, signal);
	rc = nla_put(msg, HWSIM_ATTR_TX_INFO, IEEE80211_MAX_RATES_PER_TX * sizeof(struct hwsim_tx_rate), tx_attempts);
	rc = nla_put_u64(msg, HWSIM_ATTR_COOKIE, cookie);

	if (rc != 0) {
		printf("Error filling payload\n");
		nlmsg_free(msg);
		return 0;
	}

	nl_send_auto_complete(_netlink_socket, msg);
	
	nlmsg_free(msg);

	return 1;
}


int VWifiGuest::process_messages_cb(struct nl_msg *msg, void *arg){

	forward->process_messages(msg,arg);
	return 0 ;	

}


int VWifiGuest::process_messages(struct nl_msg *msg, void *arg)
{

	int msg_len;
	struct nlattr *attrs[HWSIM_ATTR_MAX + 1];
	struct nlmsghdr * nlh;
	struct genlmsghdr * gnlh;
	struct nlmsgerr *err;
	struct ether_addr *src;
	//struct ether_addr *dst;
	unsigned int flags;
	struct hwsim_tx_rate *tx_rates;
	unsigned long cookie;
	struct hwsim_tx_rate tx_attempts[IEEE80211_MAX_RATES_PER_TX];
	int round;
	int tx_ok;
	int counter;
	int signal;
	char *data;
	struct ether_addr framesrc;
	struct ether_addr framedst;

	char addr[18];
	//int bytes;

	nlh = nlmsg_hdr(msg);
	gnlh = (struct genlmsghdr *) nlmsg_data(nlh);
	memset(addr, 0, 18);

	/* get message length needed for vsock sending */
	msg_len = nlh->nlmsg_len;

	if (nlh->nlmsg_type != m_family_id) 
		return 1;
	
	if (nlh->nlmsg_type == NLMSG_ERROR) {
		err = (struct nlmsgerr *) nlmsg_data(nlh);
		return err->error ;
	}


	if (gnlh->cmd == HWSIM_CMD_GET_RADIO) {
	
		std::cout << "HWSIM_CMD_GET_RADIO" << std::endl;
		return 1;
	}

	/* ignore if anything other than a frame
	do we need to free the msg? */
	if (!(gnlh->cmd == HWSIM_CMD_FRAME))
		return 1;

	/* processing original HWSIM_CMD_FRAME */
	genlmsg_parse(nlh, 0, attrs, HWSIM_ATTR_MAX, NULL);

	/* this check was duplicated below in a second if statement, now gone */
	if (!(attrs[HWSIM_ATTR_ADDR_TRANSMITTER]))
		return 1;


	/* this check if signal attr is present */
	//if (!(attrs[HWSIM_ATTR_SIGNAL]))
	//	std::cerr << "signal attr is not present" << std::endl;

	//if (!(attrs[HWSIM_ATTR_ADDR_RECEIVER]))

	//	std::cerr << "hwsim dst mac  (hwsim id) is not present" << std::endl ;

	/* we get hwsim mac (id)*/
	src = (struct ether_addr *)nla_data(attrs[HWSIM_ATTR_ADDR_TRANSMITTER]);

	flags = nla_get_u32(attrs[HWSIM_ATTR_FLAGS]);
	tx_rates = (struct hwsim_tx_rate *)nla_data(attrs[HWSIM_ATTR_TX_INFO]);
	cookie = nla_get_u64(attrs[HWSIM_ATTR_COOKIE]);

	round = 0;
	tx_ok = 0;

	/* We prepare the tx_attempts struct */
	set_all_rates_invalid(tx_attempts);

	while (round < IEEE80211_MAX_RATES_PER_TX &&  tx_rates[round].idx != -1 && tx_ok != 1) {

		counter = 1;

		/* tx_rates comes from the driver...
		 * that means that the receiving ends gets this info
		 * and can use it
		 */

		/* Set rate index and flags used for this round */
		tx_attempts[round].idx = tx_rates[round].idx;

		while (counter <= tx_rates[round].count && tx_ok != 1) {
			tx_attempts[round].count = counter;
			counter++;
		}
		round++;
	}

	/* round -1 is the last element of the array */
	/* this is the signal sent to the sender, not the receiver */
	signal = -10;
	
	/* Let's flag this frame as ACK'ed */
	/* whatever that means... */
	flags |= HWSIM_TX_STAT_ACK;
	
	/* this has to be an ack the driver expects */
	/* what does the driver do with these values? can i remove them? */
	send_tx_info_frame_nl(src, flags, signal, tx_attempts,cookie);

	/*
	 * no need to send a tx info frame indicating failure with a
	 * signal of 0 - that was done in the tx code i took this from
	 * if i check for ack messages than i could add a failure message
	 */

	/* we are now done with our code addition which sends the ack */

	/* we get the attributes*/
	data = (char *)nla_data(attrs[HWSIM_ATTR_FRAME]);

	/* copy source address from frame */
	/* if we rebuild the nl msg, this can change */
	memcpy(&framesrc, data + 10, ETH_ALEN);

	/* copy dst address from frame */
	memcpy(&framedst, data + 4, ETH_ALEN);

	/* compare tx src to frame src, update TX src ATTR in msg if needed */
	/* if we rebuild the nl msg, this can change */
	if (memcmp(&framesrc, src, ETH_ALEN) != 0) {

#ifdef _DEBUG	
		std::cout << "updating the TX src ATTR" << std::endl ; 
#endif
		/* copy dest address from frame to nlh */
		memcpy((char *)nlh + 24, &framesrc, ETH_ALEN);
	}



	/* we iterate the _list_winterfaces list (local wireless network devices) and see if machwsim corresponding toframesrc is set ( != 0). The value must be src */ 
/*	std::vector<WirelessDevice> inets = _list_winterfaces.list_devices();

	mac_address_to_string(addr, src);
	std::cout << "hwsim src :" <<  addr << std::endl;
	
	
	for (auto & inet : inets)
	{
		struct ether_addr macaddr = inet.getMacaddr();
		if(std::memcmp(&macaddr,&framesrc,6) == 0){
		
			struct ether_addr zeroid = {0x00,0X00,0x00,0X00,0x00,0X00};
			struct ether_addr machwsim = inet.getMachwsim();
			if(std::memcmp(&machwsim,&zeroid,6) == 0){
		
				inet.setMachwsim((*src));
				std::cout << inet << std::endl ;	
				break ;
			}
		}
	}
*/
	

	/* here code of  to send (char *)nlh with  msg_len as size*/ 
	int value=_vsocket.Send((char*)nlh,msg_len);
	
	if( value == SOCKET_ERROR )
	
	{
		std::cout<<"socket.Send error"<<std::endl;
		return 1;
	}


	return 0 ;
}


int VWifiGuest::send_register_msg()
{
	struct nl_msg *msg;

	msg = nlmsg_alloc();

	if (!msg) {
		std::cout << "Error allocating new message MSG!" << std::endl ;
		return 0;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, m_family_id,0, NLM_F_REQUEST, HWSIM_CMD_REGISTER, VERSION_NR);

	if (nl_send_auto(_netlink_socket, msg) < 0)
	{
		nlmsg_free(msg);
		return 0 ;
	}

	nl_complete_msg(_netlink_socket,msg);
	
	if (nl_send(_netlink_socket, msg) < 0)
	{
		nlmsg_free(msg);
		return 0 ;
	}

	//nl_send_auto_complete(_netlink_socket, msg);
	nlmsg_free(msg);

	return 1;
}

int VWifiGuest::send_get_info_radio_msg()
{
	struct nl_msg *msg;


	msg = nlmsg_alloc();

	if (!msg) {
		std::cout << "Error allocating new message MSG!" << std::endl ;
		return 0;
	}


	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, m_family_id,0, NLM_F_REQUEST, HWSIM_CMD_GET_RADIO, VERSION_NR);

	if (nl_send_auto(_netlink_socket, msg) < 0)
	{
		nlmsg_free(msg);
		return 0 ;
	}

	nl_complete_msg(_netlink_socket,msg);
	
	if (nl_send(_netlink_socket, msg) < 0)
	{
		nlmsg_free(msg);
		return 0 ;
	}

	std::cout << "send get info cmd " << std::endl ;

	nlmsg_free(msg);

	return 1;
}



// free better _cb and _netlink_socket
// improve the stoping process
int VWifiGuest::init_netlink(void)
{

	int nlsockfd;
	struct timeval tv;


//	_cb = nl_cb_alloc(NL_CB_DEBUG);
	_cb = nl_cb_alloc(NL_CB_CUSTOM);
	
	if (!_cb) {
		std::cerr << "Error allocating netlink callbacks" << std::endl ;
		return 0;
	}

	_netlink_socket = nl_socket_alloc_cb(_cb);
	if (!_netlink_socket) {
		std::cerr << "Error allocationg netlink socket" << std::endl;
		nl_cb_put(_cb);
		return 0;
	}

	/* disable auto-ack from kernel to reduce load */
	nl_socket_disable_auto_ack(_netlink_socket);
	
	if(genl_connect(_netlink_socket) < 0){

		nl_close(_netlink_socket);
		nl_socket_free(_netlink_socket);
		nl_cb_put(_cb);

		return 0 ;
	}

	m_family_id = genl_ctrl_resolve(_netlink_socket, "MAC80211_HWSIM");


	while (m_family_id < 0 ) {

#ifdef _DEBUG
		std::cout << "Family MAC80211_HWSIM not registered" << std::endl ;
#endif

		using namespace  std::chrono_literals;
		std::this_thread::sleep_for(1s);

		m_family_id = genl_ctrl_resolve(_netlink_socket, "MAC80211_HWSIM");
	}


	nl_cb_set(_cb, NL_CB_MSG_IN, NL_CB_CUSTOM, &process_messages_cb, NULL);
	nlsockfd = nl_socket_get_fd(_netlink_socket);

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	if (setsockopt(nlsockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {

		nl_close(_netlink_socket);
		nl_socket_free(_netlink_socket);
		nl_cb_put(_cb);
		perror("setsockopt");
	}

	return 1;
}



int VWifiGuest::send_cloned_frame_msg(struct ether_addr *dst, char *data, int data_len,int rate_idx, int signal, uint32_t freq)
{
	int rc;
	struct nl_msg *msg;

	msg = nlmsg_alloc();

	if (!msg) {
		std::cout << "Error allocating new message MSG!" << std::endl ;
		nlmsg_free(msg);
		return 0 ;
	}
	if (m_family_id < 0){
		nlmsg_free(msg);
		return 0 ;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, m_family_id, 0, NLM_F_REQUEST, HWSIM_CMD_FRAME, VERSION_NR);

	rc = nla_put(msg, HWSIM_ATTR_ADDR_RECEIVER, sizeof(struct ether_addr), dst);
	rc = nla_put(msg, HWSIM_ATTR_FRAME, data_len, data);
	rc = nla_put_u32(msg, HWSIM_ATTR_RX_RATE, rate_idx);
	rc = nla_put_u32(msg, HWSIM_ATTR_SIGNAL, signal);
	if (freq)
		rc = nla_put_u32(msg, HWSIM_ATTR_FREQ, freq);
	/* this signal rate will not match the signal acked to the sender
	 * unless we set the same rate in both functions. normally,
	 * the calling function determines this signal, and could
	 * send the info back to the transmitting radio via wmasterd
	 */

	if (rc != 0) {
		std::cout << "Error filling payload" << std::endl;
		nlmsg_free(msg);
		return 0 ;
	}


	if (nl_send_auto(_netlink_socket, msg) < 0)
	{
		nlmsg_free(msg);
		return 0 ;
	}

	nl_complete_msg(_netlink_socket,msg);
	
	if (nl_send(_netlink_socket, msg) < 0)
	{
		nlmsg_free(msg);
		return 0 ;
	}

	nlmsg_free(msg);
	
	return 1;
}




void VWifiGuest::recv_from_server(){


	char buf[1024];
	int bytes;
	
	struct nlmsghdr *nlh;
	struct genlmsghdr *gnlh;
	struct nlattr *attrs[HWSIM_ATTR_MAX + 1];
	uint32_t freq;
	unsigned int data_len = 0;
	char *data;
	int rate_idx;
	int signal;
	char addr[18];
	struct ether_addr framedst;

	signal = -10;
	rate_idx = 7;


	/* receive bytes packets from server and store them in buf */
	bytes=_vsocket.Read(buf,sizeof(buf));
	if( bytes == SOCKET_ERROR )
	{
	//	std::cerr<<"socket.Read error"<<std::endl;
		return ;
	}

	
	/* netlink header */
	nlh = (struct nlmsghdr *)buf;

	/* generic netlink header */
	gnlh = (struct genlmsghdr*)nlmsg_data(nlh);


	/* exit if the message does not contain frame data */
	if (gnlh->cmd != HWSIM_CMD_FRAME) {
	
		std::cerr << "Error - received no frame data in message" << std::endl;
		return ;
	}

	/* we get the attributes*/
	genlmsg_parse(nlh, 0, attrs, HWSIM_ATTR_MAX, NULL);

	/* we get frequence */
	if (attrs[HWSIM_ATTR_FREQ])
		freq = nla_get_u32(attrs[HWSIM_ATTR_FREQ]);
	else
		freq = 0;

#ifdef _DEBUG

	std::cout << "freq : " << freq << std::endl ;
#endif

	/*  ignore HWSIM_CMD_TX_INFO_FRAME for now */
	if (gnlh->cmd == HWSIM_CMD_TX_INFO_FRAME) {
		
		std::cerr << "Ignoring HWSIM_CMD_TX_INFO_FRAME" << std::endl;
		return ;
	}

	if (!attrs[HWSIM_ATTR_ADDR_TRANSMITTER]) {

		std::cerr << "Error - message does not contain tx address" << std::endl;
		return;
	}
	
	data_len = nla_len(attrs[HWSIM_ATTR_FRAME]);
	data = (char *)nla_data(attrs[HWSIM_ATTR_FRAME]);


	/* we extract and handle a distance here */


#ifdef _DEBUG


	struct ether_addr *src = nullptr;
	struct ether_addr framesrc;
	
	/* copy hwsim id src */
	src = (struct ether_addr *)nla_data(attrs[HWSIM_ATTR_ADDR_TRANSMITTER]);
	mac_address_to_string(addr, src);
	std::cout << "src hwsim: " << addr << std::endl ;

	/* copy mac src  address from frame */
	memcpy(&framesrc, data + 10, ETH_ALEN);
	mac_address_to_string(addr, &framesrc);
	std::cout << "frame src:" << addr << std::endl;
#endif

	/* copy mac dst address from frame */
	memcpy(&framedst, data + 4, ETH_ALEN);

#ifdef _DEBUG
	mac_address_to_string(addr, &framedst);
	std::cout << "frame dst:" << addr << std::endl;
#endif


	std::vector<WirelessDevice> inets = _list_winterfaces.list_devices();

	for (auto & inet : inets)
	{
		struct ether_addr macdsthwsim = inet.getMachwsim();
		
		mac_address_to_string(addr, &macdsthwsim);

		send_cloned_frame_msg(&macdsthwsim, data, data_len,rate_idx, signal, freq);

	}

}



void VWifiGuest::recv_msg_from_hwsim_loop_start(){


	std::cout << _list_winterfaces << std::endl ; 

	
	/* loop for waiting  incoming msg from hwsim driver*/
	while (true) {


		if(!started())
			break ;
	
		nl_recvmsgs_default(_netlink_socket);
	}

	thread_dead();

}




void VWifiGuest::recv_msg_from_server_loop_start(){

	while(true){

		if(!started())
			break ;
	
		recv_from_server();
	}

	thread_dead();
}


int VWifiGuest::init(){

	/* init netlink will loop until driver is loaded */
	if ( ! init_netlink()){
		
		std::cout << "ERROR: could not initialize netlink" << std::endl;
		return 0 ;
	} 

	/* Send a register msg to the kernel */
	if (!send_register_msg()){
	
		nl_close(_netlink_socket);
		nl_socket_free(_netlink_socket);
		nl_cb_put(_cb);
		return 0 ;
	}

	_mutex_initialized.lock();
	_initialized = true ;
	_mutex_initialized.unlock();

	
	std::cout << "Registered with family MAC80211_HWSIM" << std::endl;

	return 1 ;
}



int VWifiGuest::start(){


	// check if initialized here, if we forget calling init function before ??
	if(! initialized()){
		
		std::cerr << "You must call init() function before" << std::endl ;
		return 0 ;
	}

	// check _stoped instead m_started or check all_thread_dead ?
	if( ! stopped())
	{
		std::cerr << "vwifi-guest is already started" <<  std::endl ;
		return 0 ;
	}

	MonitorWirelessDevice * monwireless = nullptr ;

	try{

		/** make it attribute member if you would use it elsewhere */	
		monwireless = new MonitorWirelessDevice() ;
	
		monwireless->setNewInetCallback([this](WirelessDevice wd) { return handle_new_winet_notification(wd);});
		monwireless->setDelInetCallback([this](WirelessDevice wd) { return handle_del_winet_notification(wd);});
		monwireless->setInitInetCallback([this](WirelessDevice wd) { return handle_init_winet_notification(wd);});
	
		monwireless->start();
		
		/* get initial wireless network interfaces created when we called sudo modprobe mac80211_hwsim */
		monwireless->get_winterface_infos();

	}catch ( const std::exception & e){
	
		std::cerr << e.what() << std::endl ;
		return 0 ;
	
	}

	/*connect to vsock/tcp server */
#ifdef _USE_VSOCK_BY_DEFAULT_
	if( ! _vsocket.Connect(WIFI_PORT) )
#else
	if( ! _vsocket.Connect(ADDRESS_IP,WIFI_PORT) )
#endif
	{
		std::cout<<"socket.Connect error"<<std::endl;
		return 0;
	}

	/* we can also call this in constructor ? */
	_vsocket.SetBlocking(0);

	std::cout << "Connection to Server Ok" << std::endl;


	_mutex_all_thread_dead.lock();
	_all_thread_dead = 0;
	_mutex_all_thread_dead.unlock();

	
	/* start thread that handle incoming msg from hwsim driver */
	std::thread hwsimloop(&VWifiGuest::recv_msg_from_hwsim_loop_start,this);

	/* start thread that handle incoming msg from tcp or vsock connection to server */
	std::thread serverloop(&VWifiGuest::recv_msg_from_server_loop_start,this);

	m_mutex_ctrl_run.lock();
	m_started = true ;
	m_mutex_ctrl_run.unlock();

	_mutex_stopped.lock();
	_stopped = false ;
	_mutex_stopped.unlock();


	hwsimloop.join();
	serverloop.join();

	delete monwireless ;

	return 1;
}


int VWifiGuest::stop(){

	if( stopped())
		return 0 ;

	m_mutex_ctrl_run.lock();
	m_started = false ;
	m_mutex_ctrl_run.unlock();

	_vsocket.Close();

	while(!all_thread_dead(2));

	_mutex_stopped.lock();
	_stopped = true ;
	_mutex_stopped.unlock();

	return 0 ;	
}


void VWifiGuest::clean_all(){

	nl_close(_netlink_socket);
	nl_socket_free(_netlink_socket);
	nl_cb_put(_cb);
}


VWifiGuest::VWifiGuest()  {

	/* allows calls from  static callback to non static member function */ 
	forward = new vwifiguest::CallFromStaticFunc(this);
}



VWifiGuest::~VWifiGuest(){

	std::cout << __func__ << std::endl ;

	if (started())
		stop();

	clean_all();
	
	if (forward)
		delete forward ;
}


bool VWifiGuest::all_thread_dead(int nb_thread){

	_mutex_all_thread_dead.lock();

	if(_all_thread_dead == nb_thread){

		_mutex_all_thread_dead.unlock();
		return true ;
	}
		
	_mutex_all_thread_dead.unlock();

	return false ;
}



void VWifiGuest::thread_dead(){

	_mutex_all_thread_dead.lock();
	_all_thread_dead += 1;
	_mutex_all_thread_dead.unlock();
}

bool VWifiGuest::started(){

	m_mutex_ctrl_run.lock();
		
		if(! m_started){
			m_mutex_ctrl_run.unlock();
			return false ;

		}
		
	m_mutex_ctrl_run.unlock();

	return true ;

}

bool VWifiGuest::stopped(){

	_mutex_stopped.lock();
		
		if(! _stopped){
			_mutex_stopped.unlock();
			return false ;

		}
		
	_mutex_stopped.unlock();

	return true ;

}



bool VWifiGuest::initialized(){

	_mutex_initialized.lock();
		
		if(! _initialized){
			_mutex_initialized.unlock();
			return false ;

		}
		
	_mutex_initialized.unlock();

	return true ;

}



void VWifiGuest::mac_address_to_string(char *address, struct ether_addr *mac)
{
	sprintf(address, "%02X:%02X:%02X:%02X:%02X:%02X",
		mac->ether_addr_octet[0], mac->ether_addr_octet[1], mac->ether_addr_octet[2],
		mac->ether_addr_octet[3], mac->ether_addr_octet[4], mac->ether_addr_octet[5]);
}



void VWifiGuest::handle_new_winet_notification(WirelessDevice wirelessdevice){


	std::cout << "Change in wireless configuration of : " <<  wirelessdevice << std::endl ;

}

void VWifiGuest::handle_del_winet_notification(WirelessDevice wirelessdevice){


	std::cout << "Delete wireless intarface : " << wirelessdevice << std::endl ;

}

void VWifiGuest::handle_init_winet_notification(WirelessDevice wirelessdevice){


	_list_winterfaces.add_device(wirelessdevice);

}



