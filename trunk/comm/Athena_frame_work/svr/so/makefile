.PHONY: all clean

TOP_DIR = /data/src/yutao/Athena/trunk

include $(TOP_DIR)/makeinclude/makefile.include

INC_SVR_FRAME=-I../src/
LIB_SVR_FRAME=-L../lib -lsvr_frame

INC=-I./ $(INC_SVR_FRAME) $(INC_JCE) $(INC_MYSQL) $(INC_OPEN_SOURCE_MYSQL)
LIB=$(LIB_SVR_FRAME) $(LIB_MYSQL) $(LIB_OPEN_SOURCE_MYSQL)

#CFLAGS := $(subst -z defs, -Wno-unused-variable,$(CFLAGS)) -m32 -fPIC -shared
CFLAGS = -m32 -fPIC -shared

BIN=libperson_manager.so
OBJS=$(patsubst %.cpp, %.o, $(wildcard *.cpp))

RED = \\e[1m\\e[31m
RESET = \\e[m
GREEN = \\e[1m\\e[32m

all:$(BIN)

$(BIN):$(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $@ $(LIB)

%.o:%.cpp
	echo -e "make $(GREEN)$@$(RESET) begin..."
	$(CXX) $(CFLAGS) $(INC) -c -g $^ -o $@
	echo -e "make $(GREEN)$@$(RESET) ok."

clean:
	@rm -rf $(OBJS) $(BIN)
	@echo -e "make clean done."
