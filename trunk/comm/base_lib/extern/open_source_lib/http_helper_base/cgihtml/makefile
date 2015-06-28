# Modified by LHX 1999/08/03
# Changed the install dir to tams/src/comm/...

# macros and variables
CC= gcc
RANLIB= ranlib
CFLAGS= -g -Wall -DUNIX #-DUPLOADDIR='"/tmp"'

OBJS = string-lib.o cgi-llist.o cgi-lib.o html-lib.o
LIB = ../lib/libcgihtml.a

# targets
$(LIB):
	rm -rf $@
	rm -rf *.o
	$(CC) $(CFLAGS) -c string-lib.c
	$(CC) $(CFLAGS) -c cgi-llist.c
	$(CC) $(CFLAGS) -c cgi-lib.c
	$(CC) $(CFLAGS) -c html-lib.c
	ar q $@ $(OBJS)
	$(RANLIB) $@

all:check $(LIB)

$(FASTCGILIB): 
	rm -rf $@
	rm -rf *.o
	$(CC) $(CFLAGS) -DFASTCGI -c string-lib.c
	$(CC) $(CFLAGS) -DFASTCGI -c cgi-llist.c
	$(CC) $(CFLAGS) -DFASTCGI -c cgi-lib.c
	$(CC) $(CFLAGS) -DFASTCGI -c html-lib.c
	ar q $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f *.o $(LIB)
