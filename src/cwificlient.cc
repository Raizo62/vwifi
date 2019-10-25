#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <netlink/route/link.h>

#include <linux/if_arp.h>
#include <linux/nl80211.h>

#include <linux/ethtool.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include "ieee80211.h"
#include "cwificlient.h"

#include <iostream>
#include <thread>
#include <cstring>

#include "config.h"
#include <unistd.h>


/* allow calling non static function from static function */
cwificlient::CallFromStaticFunc * CWifiClient::forward = nullptr ;



void CWifiClient::set_all_rates_invalid(struct hwsim_tx_rate *tx_rate)
{
	/* Set up all unused rates to be -1 */
	for (int i = 0; i < IEEE80211_MAX_RATES_PER_TX; i++) {
		tx_rate[i].idx = -1;
		tx_rate[i].count = 0;
	}
}


int CWifiClient::send_tx_info_frame_nl(struct ether_addr *src, unsigned int flags, int signal, struct hwsim_tx_rate *tx_attempts, unsigned long cookie)
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
		
		std::cerr << __func__ <<  "m_family_id < 0" << std::endl ;
		nlmsg_free(msg);
		return 0;
	}

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, m_family_id,	0, NLM_F_REQUEST, HWSIM_CMD_TX_INFO_FRAME, VERSION_NR);

	/* i have to ack the src the driver expects
	 * so there are no mac address modifications here
	 */
	rc = nla_put(msg, HWSIM_ATTR_ADDR_TRANSMITTER, sizeof(struct ether_addr), src);
	rc = nla_put_u32(msg, HWSIM_ATTR_FLAGS, flags);
	rc = nla_put_u32(msg, HWSIM_ATTR_SIGNAL, signal);
	rc = nla_put(msg, HWSIM_ATTR_TX_INFO, IEEE80211_MAX_RATES_PER_TX * sizeof(struct hwsim_tx_rate), tx_attempts);
	rc = nla_put_u64(msg, HWSIM_ATTR_COOKIE, cookie);

	if (rc != 0) {

		std::cerr << "Error filling payload" << std::endl;
		nlmsg_free(msg);
		return 0;
	}

	//nl_send_auto_complete(_netlink_socket, msg); //deprecated
	
	if (nl_send_auto(_netlink_socket, msg) < 0)
	{
		nlmsg_free(msg);
		return 0 ;
	}


	nlmsg_free(msg);

	return 1;
}


int CWifiClient::process_messages_cb(struct nl_msg *msg, void *arg){

	forward->process_messages(msg,arg);
	return 0 ;	

}


int CWifiClient::process_messages(struct nl_msg *msg, void *arg)
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

	
	/* ignore if anything other than a frame
	do we need to free the msg? */
	if (!(gnlh->cmd == HWSIM_CMD_FRAME)){
	
		std::cerr << "Not HWSIM_CMD_FRAME" << std::endl ;
		return 1;
	}
	
	/* processing original HWSIM_CMD_FRAME */
	genlmsg_parse(nlh, 0, attrs, HWSIM_ATTR_MAX, NULL);
	

	/* this check was duplicated below in a second if statement, now gone */
	if (!(attrs[HWSIM_ATTR_ADDR_TRANSMITTER]))
		return 1;

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

	/* send msg to a server */
	TPower power=10;

	WirelessDevice  dev ;
        if ( _list_winterfaces.get_device_by_mac(dev,framesrc))
	{	
		power = dev.getTxPower() / 100; // must add the remainder if not multiple of 2 
	}
	

	int value=_vsocket.Send((char*)&power,sizeof(power));

	if (value == SOCKET_DISCONNECT)
		manage_server_crash();

	if( value == SOCKET_ERROR )
	
	{
		std::cout<<"socket.Send error"<<std::endl;
		return 1;
	}

	/* send msg to a server */ 
	value=_vsocket.Send((char*)nlh,msg_len);

	if (value == SOCKET_DISCONNECT)
		manage_server_crash();

	if( value == SOCKET_ERROR )
	
	{
		std::cout<<"socket.Send error"<<std::endl;
		return 1;
	}



	return 0 ;
}


int CWifiClient::send_register_msg()
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


	//nl_send_auto_complete(_netlink_socket, msg); //deprecated
	nlmsg_free(msg);

	return 1;
}


// free better _cb and _netlink_socket
// improve the stoping process
int CWifiClient::init_netlink(void)
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


	while (m_family_id  < 0 ) {

		if ( ! started()){
			return 0 ;
		}

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



int CWifiClient::send_cloned_frame_msg(struct ether_addr *dst, char *data, int data_len,int rate_idx, int signal, uint32_t freq)
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

//	nl_send_auto_complete(_netlink_socket, msg);


	if (nl_send_auto(_netlink_socket, msg) < 0)
	{
		nlmsg_free(msg);
		return 0 ;
	}

	nlmsg_free(msg);
	
	return 1;
}




void CWifiClient::recv_from_server(){


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
	struct ether_addr framedst;

	signal = -10;
	rate_idx = 7;

	/* receive power from server and store them in power */
	TPower power;
	bytes=_vsocket.Read((char*)&power,sizeof(power));
	
	signal = power ;

	if (bytes == SOCKET_DISCONNECT)
		manage_server_crash();

	if( bytes == SOCKET_ERROR )  // bytes == 0 if non blocking socket
	{
	//	std::cerr<<"socket.Read error"<<std::endl;
		return ;
	}

	/* receive bytes packets from server and store them in buf */
	bytes=_vsocket.Read(buf,sizeof(buf));

	if (bytes == SOCKET_DISCONNECT)
		manage_server_crash();

	if( bytes == SOCKET_ERROR )  // bytes == 0 if non blocking socket
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
	char addr[18];
	
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
		
		send_cloned_frame_msg(&macdsthwsim, data, data_len,rate_idx, signal, freq);

	
	}

}


void  CWifiClient::monitor_hwsim_loop()
{
	struct nl_sock *sock;
	int family_id ;
	
	sock = nl_socket_alloc();
	genl_connect(sock);


	thread_start();
	
	/* loop for waiting  incoming msg from hwsim driver*/
	while (true) {

		if(!started())
			break ;

		using namespace  std::chrono_literals;
		std::this_thread::sleep_for(1s);

		family_id = genl_ctrl_resolve(sock, "MAC80211_HWSIM");

		if (family_id < 0) {
		
			std::cout << "Hwsim Driver unloaded" << std::endl ;
			_mutex_initialized.lock();
			_initialized = false ;
			_mutex_initialized.unlock();
		
			if(!init()){
				break;
			}
		}

		
	}

	nl_close(sock);
	nl_socket_free(sock);

	thread_dead();
}




void CWifiClient::recv_msg_from_hwsim_loop_start(){



	thread_start();
	
	/* loop for waiting  incoming msg from hwsim driver*/
	while (true) {
		
		if(!started())
			break ;
		/* added for monitor_hwsim_loop */	
		if(!initialized()){
		
			std::cout << __func__ << "driver still unloaded" << std::endl ;	
			using namespace  std::chrono_literals;
			std::this_thread::sleep_for(1s);
			continue ;
		}

		nl_recvmsgs_default(_netlink_socket);
	}

	thread_dead();

}


void CWifiClient::recv_msg_from_server_loop_start(){

	thread_start();

	while(true){

		if(!started())
			break ;

		/* added for monitor_hwsim_loop */
		if(!initialized()){
		
			std::cout <<  __func__ << "driver still unloaded" << std::endl ;	
			using namespace  std::chrono_literals;
			std::this_thread::sleep_for(1s);
			continue ;
		}
	
		recv_from_server();

	}

	thread_dead();
}

void CWifiClient::winet_update_loop(){


	thread_start();
	
	while (true) {
		
		if(!started())
			break ;
		/* added for monitor_hwsim_loop */	
		if(!initialized()){
		
			std::cout << __func__ << "driver still unloaded" << std::endl ;	
			using namespace  std::chrono_literals;
			std::this_thread::sleep_for(1s);
			continue ;
		}


		/* update txpower of all interfaces. We can do it elsewhere */

			//std::cout << "update interface :  " << inet.getIndex() << std::endl ;
			monwireless->get_winterface_infos(0); 

		using namespace  std::chrono_literals;
		std::this_thread::sleep_for(1s);


	}

	thread_dead();

}

int CWifiClient::init(){

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

// TSocket : AF_INET :
void CWifiClient::Init(const char* IP, TPort port)
{
	_vsocket.Init(ADDRESS_IP,WIFI_PORT);
}

// TSocket : AF_VSOCK :
void CWifiClient::Init(TPort port)
{
	_vsocket.Init(WIFI_PORT);
}

int CWifiClient::start(){


	if( started())
	
	{
		std::cerr << "vwifi-guest is already started" <<  std::endl ;
		return 0 ;
	}

	m_mutex_ctrl_run.lock();
	m_started = true ;
	m_mutex_ctrl_run.unlock();

	// check if initialized here, if we forget calling init function before ??
	if(! initialized()){
	
		if (!init()){
	
			m_mutex_ctrl_run.lock();
			m_started = false ;
			m_mutex_ctrl_run.unlock();
		
			return 0;
		}
	}

	//  check that  all_thread_dead in before starting ?
	if( ! stopped())
	{

		m_mutex_ctrl_run.lock();
		m_started = false ;
		m_mutex_ctrl_run.unlock();
		std::cerr << "One or several threads are running" <<  std::endl ;
		return 0 ;
	}

	try{

		/** make it attribute member if you would use it elsewhere */	
		monwireless = new MonitorWirelessDevice() ;
	
		monwireless->setNewInetCallback([this](WirelessDevice wd) { return handle_new_winet_notification(wd);});
		monwireless->setDelInetCallback([this](WirelessDevice wd) { return handle_del_winet_notification(wd);});
		monwireless->setInitInetCallback([this](WirelessDevice wd) { return handle_init_winet_notification(wd);});
	
		monwireless->start();
		
		/* get initial wireless network interfaces created when we called sudo modprobe mac80211_hwsim */
		monwireless->get_winterface_infos(0);

	}catch ( const std::exception & e){
	
		std::cerr << e.what() << std::endl ;
		return 0 ;
	
	}

	/*connect to vsock/tcp server */
	if( ! _vsocket.Connect() )
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
	std::thread hwsimloop(&CWifiClient::recv_msg_from_hwsim_loop_start,this);

	/* start thread that handle incoming msg from tcp or vsock connection to server */
	std::thread serverloop(&CWifiClient::recv_msg_from_server_loop_start,this);

	/* start thread monitoring  the starting of hwsim driver*/
	std::thread monitorloop(&CWifiClient::monitor_hwsim_loop,this);

	/* start thread updating wireless inet interfaces*/
	std::thread winterface_update_loop(&CWifiClient::winet_update_loop,this);


	_mutex_stopped.lock();
	_stopped = false ;
	_mutex_stopped.unlock();


	monitorloop.join();
	hwsimloop.join();
	serverloop.join();
	winterface_update_loop.join();

	delete monwireless ;

	return 1;
}


int CWifiClient::stop(){

	m_mutex_ctrl_run.lock();
	m_started = false ;
	m_mutex_ctrl_run.unlock();

	/* stop the retrying connection to vsock server */
	_vsocket.StopReconnect(true);

	if( stopped())
		return 0 ;


	_vsocket.Close();

	while(!all_thread_dead());

	std::cout << "int stop after kill" << std::endl ;

	_mutex_stopped.lock();
	_stopped = true ;
	_mutex_stopped.unlock();

	return 0 ;	
}


void CWifiClient::clean_all(){

	nl_close(_netlink_socket);
	nl_socket_free(_netlink_socket);
	nl_cb_put(_cb);
}

void CWifiClient::manage_server_crash(){

	std::cout << "vsock/tcp connection with  server is lost" << std::endl ;
	_vsocket.Close();

	std::cout << "Reconnecting to vsock/tcp server..." << std::endl ;

	/*connect to vsock/tcp server */
	if( ! _vsocket.Connect() )
	{
		std::cout<<"socket.Connect error"<<std::endl;
		return ;
	}

	/* we can also call this in constructor ? */
	_vsocket.SetBlocking(0);

	std::cout << "Reconnection to Server Ok" << std::endl;


}

CWifiClient::CWifiClient()  {

	/* allows calls from  static callback to non static member function */ 
	forward = new cwificlient::CallFromStaticFunc(this);
}



CWifiClient::~CWifiClient(){

	std::cout << __func__ << std::endl ;

	if (started())
		stop();

	clean_all();
	
	if (forward)
		delete forward ;
}


bool CWifiClient::all_thread_dead(){

	_mutex_all_thread_dead.lock();

	if(_all_thread_dead == 0){

		_mutex_all_thread_dead.unlock();
		return true ;
	}
		
	_mutex_all_thread_dead.unlock();

	return false ;
}



void CWifiClient::thread_dead(){

	_mutex_all_thread_dead.lock();
	_all_thread_dead -= 1;
	_mutex_all_thread_dead.unlock();
}

void CWifiClient::thread_start(){

	_mutex_all_thread_dead.lock();
	_all_thread_dead += 1;
	_mutex_all_thread_dead.unlock();
}


bool CWifiClient::started(){

	m_mutex_ctrl_run.lock();
		
		if(! m_started){
			m_mutex_ctrl_run.unlock();
			return false ;

		}
		
	m_mutex_ctrl_run.unlock();

	return true ;

}

bool CWifiClient::stopped(){

	_mutex_stopped.lock();
		
		if(! _stopped){
			_mutex_stopped.unlock();
			return false ;

		}
		
	_mutex_stopped.unlock();

	return true ;

}



bool CWifiClient::initialized(){

	_mutex_initialized.lock();
		
		if(! _initialized){
			_mutex_initialized.unlock();
			return false ;

		}
		
	_mutex_initialized.unlock();

	return true ;

}



void CWifiClient::mac_address_to_string(char *address, struct ether_addr *mac)
{
	sprintf(address, "%02X:%02X:%02X:%02X:%02X:%02X",
		mac->ether_addr_octet[0], mac->ether_addr_octet[1], mac->ether_addr_octet[2],
		mac->ether_addr_octet[3], mac->ether_addr_octet[4], mac->ether_addr_octet[5]);
}



void CWifiClient::handle_new_winet_notification(WirelessDevice wirelessdevice){


	//std::cout << "Change in wireless configuration of : " <<  wirelessdevice << std::endl ;

	/* it is necessary to do this in the case of reloading hwsim driver and not just adding wirelessdevice to _list_winterfaces */
	struct ether_addr paddr ;
	std::memset(&paddr, 0, sizeof(paddr));

	if(get_pmaddr(paddr,wirelessdevice.getName().c_str())){
      
		paddr.ether_addr_octet[0] |= 0x40 ; 
		wirelessdevice.setMachwsim(paddr); 
		_list_winterfaces.add_device(wirelessdevice);
	}


	//std::cout << __func__ << _list_winterfaces << std::endl ;


}

void CWifiClient::handle_del_winet_notification(WirelessDevice wirelessdevice){


	std::cout << "Delete wireless intarface : " << wirelessdevice << std::endl ;
	_list_winterfaces.delete_device(wirelessdevice);

}

/* called the first time we detect the interface */
void CWifiClient::handle_init_winet_notification(WirelessDevice wirelessdevice){


        struct ether_addr paddr ;
	std::memset(&paddr, 0, sizeof(paddr));

	if(get_pmaddr(paddr,wirelessdevice.getName().c_str())){
      
		paddr.ether_addr_octet[0] |= 0x40 ; 
		wirelessdevice.setMachwsim(paddr); 
		_list_winterfaces.add_device(wirelessdevice);
	}


	//std::cout << __func__ << _list_winterfaces << std::endl ;
}

/* get the permanent mac address, this function with nl_recvmsgs(wifi.nls, wifi.cb) permit change mac address before or after launching the application */ 
bool CWifiClient::get_pmaddr(struct ether_addr & paddr ,const char *ifname)

{
    int sock;
    struct ifreq ifr;
    struct ethtool_perm_addr *epmaddr;

 
    epmaddr = (ethtool_perm_addr *) malloc(sizeof(struct ethtool_perm_addr) + MAX_ADDR_LEN);
    if (!epmaddr)
    {
         perror("malloc");
         return 0;
    }
 
    std::memset(&ifr, 0, sizeof(ifr));
 
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
         free(epmaddr);
         perror("socket");
         return 0;
    }
 
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
    epmaddr->cmd = ETHTOOL_GPERMADDR;
    epmaddr->size = MAX_ADDR_LEN;
    ifr.ifr_data = (char *) epmaddr;
 
    if (ioctl(sock, SIOCETHTOOL, &ifr) == -1)
    {
         perror("ioctl");
         return 0;    
    }
    else
    {
         if (epmaddr->size != ETH_ALEN)
         {
              return 0;
         }
         else
         {
		 for(int i=0 ; i < 6 ; i++)
	 		 paddr.ether_addr_octet[i] = epmaddr->data[i] ;
             
	 }
    }
 
    free(epmaddr);
    close(sock);
 
    return 1;
}

