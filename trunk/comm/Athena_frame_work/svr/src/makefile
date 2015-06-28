.PHONY:all clean

TOP_DIR = /data/src/yutao/Athena/trunk

include $(TOP_DIR)/makeinclude/makefile.include

INC = -I./ $(INC_JCE) $(INC_MYSQL) $(INC_OPEN_SOURCE_MYSQL)
LIB=  ../lib/libsvr_frame.a

OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

CFLAGS = -m32 


all: $(LIB)

RED = \\e[1m\\e[31m
RESET = \\e[m
GREEN = \\e[1m\\e[32m

$(LIB):$(OBJS)
	@rm -rf $(LIB)
	ar q $@ $(OBJS)
	@echo -e "$(GREEN)$@$(RESET) ok"

%.o:%.cpp
	@echo -e "make $(GREEN) $@ $(RESET) begin......\c"
	$(CXX) $(INC) -c $< $(CFLAGS)
	@echo -e "$(GREEN)$@$(RESET) ok"

clean:
	@rm -rf $(OBJS) $(LIB)
	@echo "make clean done."

