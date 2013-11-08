CC=gcc
OBJS=netutil.o log.o iftap.o table.o sock.o vxlan.o net.o ctl.o main.o#test.o mpool.o 
SRCS=${OBJS:%.o=%.c}
LDLIBS=-lpthread
TARGET=vxland
#DEBUG_FLAG=-DDEBUG
CFLAGS=-Wall
CONTROLER=vxlanctl
CONTROLER_OBJS=vxlanctl.o sock.o log.o netutil.o
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

test: debug
debug:
	${MAKE} DEBUG_FLAG="-DDEBUG -g" OBJS="${OBJS} test.o"
	@cd test && ${MAKE}

clean:
	@rm -f *.o ${TARGET}
	@cd test && ${MAKE} -s clean

install:
uninstall:
