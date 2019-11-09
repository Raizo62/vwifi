#!/usr/bin/make -f
#
NAME	=	vwifi
VERSION	=	1
BINDIR	=	$(DESTDIR)/usr/local/bin
MANDIR	=	$(DESTDIR)/usr/local/man/man1

EXEC	=	vwifi-server vwifi-guest vwifi-host vwifi-ctrl vwifi-inet-monitor

SRC		=	src
OBJ		=	obj
MAN		=	man

CC		=	g++

#MODE= -O4 -Wall -DNDEBUG -fomit-frame-pointer # //////////      RELEASE WITHOUT ASSERT
MODE= -O4 -Wall -fomit-frame-pointer # //////////      RELEASE
#MODE= -g -Wall -D_DEBUG # //////////      DEBUG
#MODE= -pg # //////////      PROFILER --> view with : gprof $(NAME)

CFLAGS  +=  $(MODE)

NETLINK_FLAGS = -I/usr/include/libnl3
NETLINK_LIBS = -lnl-genl-3 -lnl-3

THREAD_LIBS = -lpthread

LIBS =

EUID	:= $(shell id -u -r)

##############################

vpath %.cc $(SRC)
vpath %.h $(SRC)

.PHONY: all clean build install man directories update

build : directories $(EXEC) # man


$(OBJ)/cscheduler.o: cscheduler.cc cscheduler.h types.h

$(OBJ)/csocket.o: csocket.cc csocket.h types.h config.h

$(OBJ)/cwirelessdevice.o: cwirelessdevice.cc cwirelessdevice.h

$(OBJ)/ccoordinate.o: ccoordinate.cc ccoordinate.h types.h

$(OBJ)/tpower.o: tpower.cc tpower.h types.h config.h

$(OBJ)/cinfosocket.o: cinfosocket.cc cinfosocket.h types.h


$(OBJ)/csocketserver.o: csocketserver.cc csocketserver.h $(OBJ)/csocket.o $(OBJ)/cinfosocket.o types.h
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<

$(OBJ)/cwifiserver.o: cwifiserver.cc cwifiserver.h $(OBJ)/csocketserver.o $(OBJ)/cinfowifi.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<

$(OBJ)/csocketclient.o: csocketclient.cc csocketclient.h $(OBJ)/csocket.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<

$(OBJ)/cinfowifi.o: cinfowifi.cc cinfowifi.h $(OBJ)/ccoordinate.o types.h
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<

$(OBJ)/cctrlserver.o: cctrlserver.cc cctrlserver.h $(OBJ)/cwifiserver.o $(OBJ)/cscheduler.o types.h
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<


$(OBJ)/cmonwirelessdevice.o: cmonwirelessdevice.cc cmonwirelessdevice.h
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<

$(OBJ)/cwificlient.o: cwificlient.cc cwificlient.h $(OBJ)/csocketclient.o $(OBJ)/cscheduler.o hwsim.h mac80211_hwsim.h ieee80211.h config.h
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<


vwifi-server : vwifi-server.cc config.h $(OBJ)/csocket.o $(OBJ)/csocketserver.o $(OBJ)/cinfosocket.o $(OBJ)/cwifiserver.o $(OBJ)/cscheduler.o $(OBJ)/cinfowifi.o $(OBJ)/ccoordinate.o $(OBJ)/cctrlserver.o $(OBJ)/tpower.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $^

vwifi-ctrl : vwifi-ctrl.cc config.h $(OBJ)/csocket.o $(OBJ)/csocketclient.o $(OBJ)/ccoordinate.o $(OBJ)/cinfowifi.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $^

vwifi-guest : vwifi-guest.cc $(OBJ)/cwirelessdevice.o $(OBJ)/cwirelessdevicelist.o  $(OBJ)/cwificlient.o $(OBJ)/cscheduler.o $(OBJ)/csocket.o $(OBJ)/csocketclient.o $(OBJ)/cmonwirelessdevice.o $(OBJ)/tpower.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) $^

vwifi-host : vwifi-host.cc $(OBJ)/cwirelessdevice.o $(OBJ)/cwirelessdevicelist.o  $(OBJ)/cwificlient.o $(OBJ)/cscheduler.o $(OBJ)/csocket.o $(OBJ)/csocketclient.o $(OBJ)/cmonwirelessdevice.o $(OBJ)/tpower.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) $^

vwifi-inet-monitor :  vwifi-inet-monitor.cc  $(OBJ)/cwirelessdevice.o $(OBJ)/cmonwirelessdevice.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) $^

$(OBJ)/%.o: %.cc
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ -c $<

$(MAN)/$(NAME).1.gz : $(MAN)/$(NAME).1
	gzip -c $(MAN)/$(NAME).1 > $(MAN)/$(NAME).1.gz

man : $(MAN)/$(NAME).1.gz

directories: $(OBJ)/.

$(OBJ)/.:
	mkdir -p $(OBJ)

all : clean build install

update :
	wget -q -N https://raw.githubusercontent.com/torvalds/linux/master/drivers/net/wireless/mac80211_hwsim.h -P $(SRC)

clean:
	-rm -f *~ $(SRC)/*~ $(MAN)/*~
	-rm -f $(EXEC) $(OBJ)/* $(MAN)/$(NAME).1.gz

install :
	chmod u+x tools/*
