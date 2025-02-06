AM_CPPFLAGS = $(NETLINK_CFLAGS) -I$(top_builddir)
AM_CXXFLAGS = -Wall -Wextra -pedantic -O3
AM_LDFLAGS = $(NETLINK_LIBS) -lpthread

bin_PROGRAMS = vwifi-server vwifi-client vwifi-ctrl vwifi-add-interfaces

vwifi_server_SOURCES = \
    vwifi-server.cc \
    ccoordinate.cc \
    cctrlserver.cc \
    cdynbuffer.cc \
    cinfosocket.cc \
    cinfowifi.cc \
    cselect.cc \
    csocket.cc \
    csocketserverfunctionitcp.cc \
    csocketserverfunctionvtcp.cc \
    csocketserver.cc \
    cwifi.cc \
    cwifiserveritcp.cc \
    cwifiserver.cc \
    cwifiservervtcp.cc \
    tools.cc

vwifi_client_SOURCES = \
    vwifi-client.cc \
    addinterfaces.cc \
    cdynbuffer.cc \
    ckernelwifi.cc \
    cmonwirelessdevice.cc \
    cselect.cc \
    csocketclientitcp.cc \
    csocketclient.cc \
    csocketclientvtcp.cc \
    csocket.cc \
    cthread.cc \
    cwificlient.cc \
    cwifi.cc \
    cwirelessdevicelist.cc \
    cwirelessdevice.cc \
    tools.cc

vwifi_ctrl_SOURCES = \
    vwifi-ctrl.cc \
    ccoordinate.cc \
    cdynbuffer.cc \
    cinfowifi.cc \
    csocketclientitcp.cc \
    csocketclient.cc \
    csocket.cc \
    tools.cc

vwifi_add_interfaces_SOURCES = \
    vwifi-add-interfaces.cc \
    addinterfaces.cc

noinst_HEADERS = \
    *.h

EXTRA_DIST = \
    mac80211_hwsim.h 