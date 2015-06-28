TOP_DIR = ../../../../..

.PHONY: all

include $(TOP_DIR)/makeinclude/makefile.include

INC = $(INC_OPEN_SOURCE_MYSQL) -I../include/

#CFLAGS+=-D_UPP_CLIENT_DEBUG_
#CFLAGS+=-D_PINBOARD_NOTIFY_CLIENT_DEBUG_
OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) 

LIB = ../lib/libAthena_mysql_client.a

# targets
$(LIB): $(OBJS)
	@rm -rf $@
	ar q $@ $(OBJS)
	
all: $(LIB)

clean:
	rm -f *.o $(LIB)
%.o:%.cpp
	@echo -e "make $(GREEN)$< $(RESET)... \c"
	$(CXX) $(CFLAGS) $(INC) -c  $<          
	@echo -e "ok."
