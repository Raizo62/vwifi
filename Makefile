#/usr/bin/make -f
#
NAME	:=	vwifi
VERSION	:=	6.1
BINDIR	:=	$(DESTDIR)/usr/local/bin
MANDIR	:=	$(DESTDIR)/usr/local/man/man1

EXEC	:=	vwifi-server vwifi-client vwifi-ctrl vwifi-add-interfaces
#EXEC	:=	vwifi-server vwifi-client vwifi-ctrl vwifi-add-interfaces vwifi-inet-monitor

SRC		:=	src
OBJ		:=	obj
MAN		:=	man

CXX		?=	g++

#STATIC	:= -static

#MODE+= -O3 -s -Wall -Wextra -pedantic -DNDEBUG # //////////      RELEASE WITHOUT ASSERT
MODE+= -O3 -s -Wall -Wextra -pedantic # //////////      RELEASE
#MODE+= -g -Wall -Wextra -pedantic -D_DEBUG # //////////      DEBUG
#MODE+= -pg # //////////      PROFILER --> view with : gprof $(NAME)

EDITOR	?=	geany

NETLINK_FLAGS_PATH := /usr/include/libnl3
NETLINK_FLAGS := -I $(NETLINK_FLAGS_PATH)
NETLINK_LIBS_PATH := .
NETLINK_LIBS := -L $(NETLINK_LIBS_PATH) -lnl-genl-3 -lnl-3

THREAD_LIBS := -lpthread

GCC_LIBS := -lstdc++ -lm

CFLAGS  += $(NETLINK_FLAGS)
LDFLAGS += $(NETLINK_LIBS) $(THREAD_LIBS) $(GCC_LIBS)

DEFS	+= -DVERSION=\"$(VERSION)\"

EUID	:= $(shell id -u -r)

##############################

vpath %.cc $(SRC)
vpath %.h $(SRC)

.PHONY: all clean build install man dep tools directories update edit cppcheck gitversion static dyn

build : directories $(EXEC) # man

include Makefile.in

# To build obj :
$(OBJ)/%.o:
	$(CXX) -o $@ $(MODE) $(CFLAGS) $(DEFS) -c $<

# To build bin :
# On OpenWRT, $(LDFLAGS) must be after $^
$(EXEC):
	$(CXX) -o $@ $(MODE) $(STATIC) $^ $(LDFLAGS)

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
	wget -q -N https://raw.githubusercontent.com/torvalds/linux/master/drivers/net/wireless/virtual/mac80211_hwsim.h -P $(SRC)

clean:
	-rm -f *~ $(SRC)/*~ $(MAN)/*~
	-rm -f $(EXEC) $(OBJ)/* $(NAME)-cppcheck.xml $(MAN)/$(NAME).1.gz

dep:
	-bash tools/Makefile.dependency.sh

edit:
	$(EDITOR) $(SRC)/* Makefile README.md &

cppcheck:
	cppcheck --verbose --enable=all --enable=style --xml $(CFLAGS) $(DEFS) -D_DEBUG $(SRC)/*.cc 2> $(NAME)-cppcheck.xml

dyn:
	@sed -i 's/^\s*STATIC\s*:=\(.*\)/#STATIC      := \1/g' Makefile

static:
	@sed -i 's/^\s*#\s*STATIC\s*:=\(.*\)/STATIC      := \1/g' Makefile

gitversion: .git
	@sed -n "s/^\(VERSION.[^\-]*\)\(-.*\)\?/\1-$(shell git log --pretty=format:"%h" -n 1)/gp" Makefile
	@sed -i "s/^\(VERSION.[^\-]*\)\(-.*\)\?/\1-$(shell git log --pretty=format:"%h" -n 1)/g" Makefile

install : build
ifneq ($(EUID),0)
	@echo "Please run 'make install' as root user"
	@exit 1
endif
	chmod +x $(EXEC)
	# Install binaire :
	mkdir -p $(BINDIR) && cp -p $(EXEC) $(BINDIR)
