.PHONY:all clean

TOP_DIR = ../../../../../

include $(TOP_DIR)/makeinclude/makefile.include

INC=-I./ $(INC_JCE) $(INC_NETWORK) $(INC_COMMLOG)
LIB= $(LIB_NETWORK) $(LIB_COMMLOG)

OBJS = $(patsubst %.cpp, %, $(wildcard *.cpp))

all: $(OBJS)

RED = \\e[1m\\e[31m
RESET = \\e[m
GREEN = \\e[1m\\e[32m

%:%.cpp
	echo -e "make $(GREEN)$@$(RESET) begin..."
	$(CXX) $(CFLAGS) $(INC) $^ -o $@ $(LIB)
	echo -e "make $(GREEN)$@$(RESET) ok."

clean:
	rm -rf $(OBJS)

