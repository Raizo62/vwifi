#/usr/bin/make -f
#
NAME	=	vwifi
VERSION	=	2.1
BINDIR	=	$(DESTDIR)/usr/local/bin
MANDIR	=	$(DESTDIR)/usr/local/man/man1

EXEC	=	vwifi-server vwifi-client vwifi-spy vwifi-ctrl
#EXEC	=	vwifi-server vwifi-client vwifi-spy vwifi-ctrl vwifi-inet-monitor

SRC		=	src
OBJ		=	obj
MAN		=	man

CC		=	g++

#MODE= -O3 -s -Wall -Wextra -pedantic -DNDEBUG # //////////      RELEASE WITHOUT ASSERT
MODE= -O3 -s -Wall -Wextra -pedantic # //////////      RELEASE
#MODE= -g -Wall -Wextra -pedantic -D_DEBUG # //////////      DEBUG
#MODE= -pg # //////////      PROFILER --> view with : gprof $(NAME)

CFLAGS  +=  $(MODE)

NETLINK_FLAGS = -I/usr/include/libnl3
NETLINK_LIBS = -lnl-genl-3 -lnl-3

THREAD_LIBS = -lpthread

LIBS =

DEFS = -DVERSION=\"$(VERSION)\"

EUID	:= $(shell id -u -r)

##############################

vpath %.cc $(SRC)
vpath %.h $(SRC)

.PHONY: all clean build install man tools directories update gitversion

build : directories $(EXEC) # man

include Makefile.in

$(OBJ)/%.o: %.cc
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) -o $@ $(NETLINK_FLAGS) -c $<

$(MAN)/$(NAME).1.gz : $(MAN)/$(NAME).1
	gzip -c $(MAN)/$(NAME).1 > $(MAN)/$(NAME).1.gz

man : $(MAN)/$(NAME).1.gz

tools :
	chmod u+x tools/*

directories: $(OBJ)/.

$(OBJ)/.:
	mkdir -p $(OBJ)

all : clean build tools install

update :
	wget -q -N https://raw.githubusercontent.com/torvalds/linux/master/drivers/net/wireless/mac80211_hwsim.h -P $(SRC)

clean:
	-rm -f *~ $(SRC)/*~ $(MAN)/*~
	-rm -f $(EXEC) $(OBJ)/* $(MAN)/$(NAME).1.gz

gitversion: .git
	@sed -n "s/^\(VERSION.[^\-]*\)\(-.*\)\?/\1-$(shell git log --format="%H" -n 1)/gp" Makefile
	@sed -i "s/^\(VERSION.[^\-]*\)\(-.*\)\?/\1-$(shell git log --format="%H" -n 1)/g" Makefile

install : build
ifneq ($(EUID),0)
	@echo "Please run 'make install' as root user"
	@exit 1
endif
	chmod +x $(EXEC)
	# Install binaire :
	mkdir -p $(BINDIR) && cp -p $(EXEC) $(BINDIR)
