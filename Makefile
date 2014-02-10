CC=gcc
OBJS=util.o netutil.o log.o tap.o table.o sock.o vxlan.o net.o cmd.o main.o#test.o mpool.o 
SRCS=${OBJS:%.o=%.c}
LDLIBS=-lpthread
TARGET=vxland
#DEBUG_FLAG=-DDEBUG
CFLAGS=-Wall
CONTROLER=vxconfig
CONTROLER_OBJS=vxconfig.o sock.o log.o netutil.o util.o
LDFLAGS=

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
#	${MAKE} DEBUG_FLAG="-DDEBUG -g" OBJS="${OBJS} test.o"
	${MAKE} DEBUG_FLAG="-DDEBUG -g" OBJS="${OBJS}"
	@cd test && ${MAKE}

clean:
	@rm -f *.o ${TARGET} ${CONTROLER}
	@cd test && ${MAKE} -s clean

install:
uninstall:
