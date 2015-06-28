TOP_DIR = ../../../../../

.PHONY: all clean

include $(TOP_DIR)/makeinclude/makefile.include
INC = -I../include

OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp)) 
LIB = ../lib/libAthena_network.1.0.a

# targets
$(LIB): $(OBJS)
	@rm -rf $@
	ar q $@ $(OBJS)
	echo -e "$(GREEN)$@$(RESET) ok"
	
all: $(LIB)

clean:
	rm -f *.o $(LIB) 

%.o:%.cpp
	@echo -e "make $(GREEN)$< $(RESET)... \c"
	$(CXX) $(CFLAGS) $(INC) -c  $<          
	@echo -e "ok."
