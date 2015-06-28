.PHONY: all clean

INC = -I../include/
LIB = ../lib/libconfig.a

OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

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

