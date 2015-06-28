.PHONY: all clean

TOP_DIR = /data/src/yutao/Athena/trunk

include $(TOP_DIR)/makeinclude/makefile.include

INC_SVR_FRAME=-I../src/
LIB_SVR_FRAME=-L../lib -lsvr_frame

INC=$(INC_SVR_FRAME)
LIB=$(LIB_SVR_FRAME) $(LIB_MYSQL) $(LIB_OPEN_SOURCE_MYSQL) -ldl

CFLAGS = -m32

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
