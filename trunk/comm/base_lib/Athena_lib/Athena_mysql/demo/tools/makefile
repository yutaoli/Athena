.PHONY: all clean

TOP_DIR=../../../../../../
include $(TOP_DIR)/makeinclude/makefile.include

INC_PERSON=-I../include/
LIB_PERSON=-L../lib -lperson

INC=$(INC_PERSON) $(INC_MYSQL) $(INC_OPEN_SOURCE_MYSQL)
LIB=$(LIB_PERSON) $(LIB_MYSQL) $(LIB_OPEN_SOURCE_MYSQL)

LIBRARY=$(patsubst %.cpp, %, $(wildcard *.cpp))
all:$(LIBRARY)

RED = \\e[1m\\e[31m
RESET = \\e[m
GREEN = \\e[1m\\e[32m

%:%.cpp
	echo -e "make $(GREEN)$@$(RESET) begin..."
	$(CXX) $(CFLAGS) $(INC) $^ -o $@ $(LIB)
	echo -e "make $(GREEN)$@$(RESET) ok."

clean:
	@rm -rf $(LIBRARY)
	@echo -e "make clean done."
