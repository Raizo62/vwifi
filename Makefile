#!/usr/bin/make -f
#
NAME	=	vwifi
VERSION	=	1
BINDIR	=	$(DESTDIR)/usr/local/bin
MANDIR	=	$(DESTDIR)/usr/local/man/man1

EXEC	=	vwifi-host-server vwifi-guest vwifi-host-test vwifi-inet-monitor

SRC		=	src
OBJ		=	obj
MAN		=	man

CC		=	g++

#MODE= -O4 -Wall -fomit-frame-pointer # //////////      RELEASE
MODE= -g -Wall -D_DEBUG # //////////      DEBUG
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


$(OBJ)/cscheduler.o: cscheduler.cc  cscheduler.h

$(OBJ)/csocket.o: csocket.cc csocket.h

$(OBJ)/csocketserver.o: csocketserver.cc csocketserver.h csocket.h

$(OBJ)/cwifiserver.o: cwifiserver.cc cwifiserver.h csocketserver.h

$(OBJ)/csocketclient.o: csocketclient.cc csocketclient.h csocket.h

$(OBJ)/cwirelessdevice.o: cwirelessdevice.cc cwirelessdevice.h 


$(OBJ)/cvwifiguest.o: cvwifiguest.cc cvwifiguest.h hwsim.h ieee80211.h
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) -c $<


vwifi-host-server : vwifi-host-server.cc vwifi-host-server.h $(OBJ)/csocket.o $(OBJ)/csocketserver.o $(OBJ)/cwifiserver.o $(OBJ)/cscheduler.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $^

vwifi-host-test : vwifi-host-test.cc vwifi-host-test.h $(OBJ)/csocket.o $(OBJ)/csocketclient.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $^

vwifi-guest : vwifi-guest.cc $(OBJ)/cwirelessdevice.o $(OBJ)/cvwifiguest.o  $(OBJ)/csocket.o $(OBJ)/csocketclient.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $(NETLINK_FLAGS) $(NETLINK_LIBS) $(THREAD_LIBS) $^

vwifi-inet-monitor :  vwifi-inet-monitor.cc  $(OBJ)/cwirelessdevice.o 
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $^


$(OBJ)/%.o: %.cc
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ -c $<

$(MAN)/$(NAME).1.gz : $(MAN)/$(NAME).1
	gzip -c $(MAN)/$(NAME).1 > $(MAN)/$(NAME).1.gz

man : $(MAN)/$(NAME).1.gz

directories: $(OBJ)/.

$(OBJ)/.:
	mkdir -p $(OBJ)

all : clean build install

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
