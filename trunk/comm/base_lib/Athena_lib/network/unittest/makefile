.PHONY: all clean

TOP_DIR= ../../../../../

include $(TOP_DIR)/makeinclude/makefile.include

INC = $(INC_NETWORK)
LIB = $(LIB_NETWORK)

BINARY = $(patsubst %.cpp,%,$(wildcard *.cpp))

all: $(BINARY)

RED = \\e[1m\\e[31m
RESET = \\e[m
GREEN = \\e[1m\\e[32m

%:%.cpp
	@echo -e "make $(GREEN) $@ $(RESET) begin......\c"
	$(CXX) $(INC) $< -o $@ $(CFLAGS) $(LIB)
	@echo -e "$(GREEN)$@$(RESET) ok"

clean:
	@rm -rf ${BINARY}
	@echo "make clean done."

