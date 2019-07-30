#!/usr/bin/make -f
#
NAME	=	vwifi
VERSION	=	1
BINDIR	=	$(DESTDIR)/usr/local/bin
MANDIR	=	$(DESTDIR)/usr/local/man/man1

EXEC	=	vwifi-server vwifi-guest vwifi-ctrl vwifi-inet-monitor

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

.PHONY: all clean build install man directories

build : directories $(EXEC) # man


$(OBJ)/cscheduler.o: cscheduler.cc cscheduler.h types.h config.h

$(OBJ)/csocket.o: csocket.cc csocket.h types.h config.h

$(OBJ)/csocketserver.o: csocketserver.cc csocketserver.h csocket.h types.h

$(OBJ)/cwifiserver.o: cwifiserver.cc cwifiserver.h csocketserver.h cinfowifi.h

$(OBJ)/csocketclient.o: csocketclient.cc csocketclient.h csocket.h

$(OBJ)/cwirelessdevice.o: cwirelessdevice.cc cwirelessdevice.h 

$(OBJ)/cinfowifi.o: cinfowifi.cc cinfowifi.h ccoordinate.h types.h

$(OBJ)/ccoordinate.o: ccoordinate.cc ccoordinate.h types.h

$(OBJ)/cctrlserver.o: cctrlserver.cc cctrlserver.h cwifiserver.h cscheduler.h types.h

$(OBJ)/tpower.o: tpower.cc tpower.h types.h


$(OBJ)/cmonwirelessdevice.o: cmonwirelessdevice.cc cmonwirelessdevice.h
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<

$(OBJ)/cvwifiguest.o: cvwifiguest.cc cvwifiguest.h hwsim.h ieee80211.h config.h
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<


vwifi-server : vwifi-server.cc config.h $(OBJ)/csocket.o $(OBJ)/csocketserver.o $(OBJ)/cwifiserver.o $(OBJ)/cscheduler.o $(OBJ)/cinfowifi.o $(OBJ)/ccoordinate.o $(OBJ)/cctrlserver.o $(OBJ)/tpower.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $^

vwifi-ctrl : vwifi-ctrl.cc config.h $(OBJ)/csocket.o $(OBJ)/csocketclient.o $(OBJ)/ccoordinate.o $(OBJ)/cinfowifi.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $^

vwifi-guest : vwifi-guest.cc $(OBJ)/cwirelessdevice.o $(OBJ)/cwirelessdevicelist.o  $(OBJ)/cvwifiguest.o  $(OBJ)/csocket.o $(OBJ)/csocketclient.o $(OBJ)/cmonwirelessdevice.o $(OBJ)/tpower.o
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

all : clean build # install

clean:
	-rm -f *~ $(SRC)/*~ $(MAN)/*~
	-rm -f $(EXEC) $(OBJ)/* $(MAN)/$(NAME).1.gz

install : $(EXEC) man
ifneq ($(EUID),0)
	@echo "Please run 'make install' as root user"
	@exit 1
endif
	chmod +x $(EXEC)
	# Install binaire :
	mkdir -p $(BINDIR) && cp -p $(EXEC) $(BINDIR)
	# Install mapage :
	mkdir -p $(MANDIR) && cp $(MAN)/$(NAME).1.gz $(MANDIR)/$(NAME).1.gz
