CC=gcc
OBJS=netutil.o log.o iftap.o table.o sock.o vxlan.o net.o main.o#test.o mpool.o 
SRCS=${OBJS:%.o=%.c}
LDLIBS=-lpthread
TARGET=vxland
#DEBUG_FLAG=-DDEBUG
CFLAGS=-Wall -g
CONTROLER=vxlanctl
CONTROLER_OBJS=vxlanctl.o sock.o log.o netutil.o

.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBUG_FLAG} -c $<

.PHONY: all clean test

all:${TARGET} ${CONTROLER}

${TARGET}:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

${CONTROLER}:${CONTROLER_OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

debug:
	${MAKE} DEBUG_FLAG="-DDEBUG" OBJS="${OBJS} test.o"

clean:
	@rm -f *.o ${TARGET}
