#!/usr/bin/make -f
#
NAME	=	vwifi
VERSION	=	1
BINDIR	=	$(DESTDIR)/usr/local/bin
MANDIR	=	$(DESTDIR)/usr/local/man/man1

EXEC	=	vwifi-host-server vwifi-guest

SRC		=	src
OBJ		=	obj
MAN		=	man

CC		=	g++

MODE= -O4 -Wall -fomit-frame-pointer # //////////      RELEASE
#MODE= -g -Wall -D_DEBUG # //////////      DEBUG
#MODE= -pg # //////////      PROFILER --> view with : gprof $(NAME)

CFLAGS  +=  $(MODE)

LIBS =

EUID	:= $(shell id -u -r)

##############################

vpath %.cc $(SRC)
vpath %.h $(SRC)
vpath %.o $(OBJ)

.PHONY: all clean build install man directories

build : directories $(EXEC) # man

cscheduler.o: cscheduler.cc  cscheduler.h

cvsocket.o: cvsocket.cc cvsocket.h

cvsocketserver.o: cvsocketserver.cc cvsocketserver.h cvsocket.h

cvsocketclient.o: cvsocketclient.cc cvsocketclient.h cvsocket.h

vwifi-host-server : vwifi-host-server.cc vwifi-host-server.h $(OBJ)/cvsocket.o $(OBJ)/cvsocketserver.o $(OBJ)/cscheduler.o
	$(CC) $(CFLAGS) $(DEFS) $(LDFLAGS) $(LIBS) -o $@ $^

vwifi-guest : vwifi-guest.cc vwifi-guest.h $(OBJ)/cvsocket.o $(OBJ)/cvsocketclient.o
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
