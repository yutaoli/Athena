TOP_DIR=../../../../../

include $(TOP_DIR)/makeinclude/makefile.include

TLIB_HEADER_PATH= -I./ $(INC_HTTP_HELPER_BASE)

CC= gcc
TLIB=	../lib/libtlib.a

OBJS=	tlib_str.o tlib_dbf.o tlib_mail.o tlib_ini.o tlib_tools.o tlib_html.o \
		tlib_cfg.o tlib_log.o tlib_shm.o tlib_lck.o tlib_cgi.o tlib_mem.o \
		tlib_sock.o tlib_warn.o tlib_warn_log.o tlib_report.o tlib_ip_limit.o \
		md5c.o md5.o

CFLAGS= -g

RED = \\e[1m\\e[31m
RESET = \\e[m
GREEN = \\e[1m\\e[32m

.c.o:
	$(CC) $(CFLAGS) $(TLIB_HEADER_PATH) -c $<
	
$(TLIB): $(OBJS)
	ar q $(TLIB) $(OBJS)
	@echo -e "$(GREEN)$@$(RESET) ok."

all:check $(TLIB)

clean:
	rm -rf $(TLIB)
	rm -f *.o
