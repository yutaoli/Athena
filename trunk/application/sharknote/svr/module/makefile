.PHONY: all clean
TOP_DIR=../../../../

include $(TOP_DIR)/makeinclude/makefile.include

CC = gcc
CXX = g++
CFLAGS  = -g -pipe  -export-dynamic  -Wall -Werror
LIB_SB  = -ldl -lrt 

INC = $(INC_COMMLOG) $(INC_CONFIG)
LIB = $(LIB_COMMLOG) $(LIB_CONFIG)

CPP_FILE = $(wildcard ./*.cpp)
BINARY = $(patsubst ./%.cpp, ./%, $(CPP_FILE))

all: $(BINARY)

%:%.cpp
	$(CXX) $(CFLAGS) $(INC)  $^ -o $@ $(LIB)
	@echo -e "$(GREEN)$@$(RESET) ok."


clean:
	rm -f $(BINARY)
