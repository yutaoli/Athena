.PHONY: all clean

TOP_DIR= ../../../../../../

include $(TOP_DIR)/makeinclude/makefile.include

INC = $(INC_MYSQL) $(INC_OPEN_SOURCE_MYSQL) -I../include/

OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))
LIB = ../lib/libperson.a

all: $(LIB)

RED = \\e[1m\\e[31m
RESET = \\e[m
GREEN = \\e[1m\\e[32m

$(LIB):$(OBJS)
	@rm -rf $(LIB)
	ar q $@ $(OBJS)

%.o:%.cpp
	@echo -e "make $(GREEN) $@ $(RESET) begin......\c"
	$(CXX) $(INC) -c $< $(CFLAGS)
	@echo -e "$(GREEN)$@$(RESET) ok"

clean:
	@rm -rf $(OBJS) $(LIB)
	@echo "make clean done."

