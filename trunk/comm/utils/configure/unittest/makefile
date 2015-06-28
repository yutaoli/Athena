.PHONY: all clean

TOP_DIR=../../../../
include $(TOP_DIR)/makeinclude/makefile.include

INC = -I../include/ $(INC_COMMLOG)
LIB = -L../lib/ -lconfig $(LIB_COMMLOG)

BINARY = $(patsubst %.cpp, %, $(wildcard *.cpp))

all: $(BINARY)
ED = \\e[1m\\e[31m
RESET = \\e[m
GREEN = \\e[1m\\e[32m

clean:
	@rm -rf ${BINARY}
	@for xfile in $(BINARY); \
        do \
            (rm -rf xfile); \
        done
	@echo "make clean done."

%:%.cpp
	@echo -e "Make $(GREEN)$@$(RESET) begin......\c"
	$(CXX) $(INC) $< -o $@ $(CFLAGS) ${LIB} 
	@echo -e "$(GREEN)$@$(RESET) ok"
	

