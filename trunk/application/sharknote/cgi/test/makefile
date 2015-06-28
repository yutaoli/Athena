TOP_DIR=../../../../

include $(TOP_DIR)/makeinclude/makefile.include

CC = gcc
CXX = g++
#CFLAGS  = -g -Wall -pipe -z defs -fPIC -shared -export-dynamic  
CFLAGS  = -g -pipe  -fPIC -shared -export-dynamic  -Wall -Werror
LIB_SB  = -ldl -lrt 

INC = $(INC_HTTP_HELPER) $(INC_HTTP_HELPER_BASE) $(INC_JSON) $(INC_COMMLOG) $(INC_ATHENA_NETWORK_PARSER) $(INC_JCE) $(INC_NETWORK)
LIB = $(LIB_HTTP_HELPER) $(LIB_HTTP_HELPER_BASE) $(LIB_JSON) $(LIB_COMMLOG) $(LIB_JCE) $(LIB_NETWORK)

CPP_FILE = $(wildcard ./*.cpp)
BINARY = $(patsubst ./%.cpp, ./%, $(CPP_FILE))

all: $(BINARY)

%:%.cpp
	$(CXX) $(CFLAGS) $(INC)  $^ -o $@ $(LIB)
	@echo -e "$(GREEN)$@$(RESET) ok."


clean:
	rm -f $(BINARY)
