CC=gcc
OBJS=util.o netutil.o log.o tap.o table.o sock.o vxlan.o net.o cmd.o config.o main.o#test.o mpool.o 
SRCS=${OBJS:%.o=%.c}
LDLIBS=-lpthread
TARGET=vxland
#DEBUG_FLAG=-DDEBUG
CFLAGS=-Wall
CONTROLER=vxconfig
CONTROLER_OBJS=vxconfig.o sock.o log.o netutil.o util.o
LDFLAGS=

PREFIX=/usr/local/bin
SCRIPT_DIR=script
LSB_SCRIPT=vxland
INIT_DIR=/etc/init.d
CONFIG_SRC=./conf/vxlan.conf
CONFIG_DST=/etc/vxlan.conf

.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBUG_FLAG} -c $<

.PHONY: all debug clean test install uninstall

all:${TARGET} ${CONTROLER}

${TARGET}:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

${CONTROLER}:${CONTROLER_OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

debug:
	${MAKE} DEBUG_FLAG="-g" OBJS="${OBJS}"

netdebug:
#	${MAKE} DEBUG_FLAG="-DDEBUG -g" OBJS="${OBJS} test.o"
	${MAKE} DEBUG_FLAG="-DDEBUG -g" OBJS="${OBJS}"
	@cd test && ${MAKE}

clean:
	@rm -f *.o ${TARGET} ${CONTROLER}
	@cd test && ${MAKE} -s clean

.PHONY: install-bin install-conf install-script

install-bin:all
	install -p -m 755 ${TARGET} ${PREFIX}/${TARGET}
	install -p -m 755 ${CONTROLER} ${PREFIX}/${CONTROLER}

install-script:
	install -p -m 755 ${SCRIPT_DIR}/${LSB_SCRIPT} ${INIT_DIR}/${LSB_SCRIPT}

install-conf:
	install -p -m 644 ${CONFIG_SRC} ${CONFIG_DST}

install: install-bin install-script install-conf

uninstall:
	@rm -f ${PREFIX}/${TARGET}
	@rm -f ${PREFIX}/${CONTROLER}
	@rm -f ${INIT_DIR}/${LSB_SCRIPT}
