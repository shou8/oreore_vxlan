CC=gcc
OBJS=main.o netutil.o log.o iftap.o table.o net.o vxlan.o #test.o mpool.o 
SRCS=${OBJS:%.o=%.c}
LDLIBS=-lpthread
TARGET=vxland
#DEBUG_FLAG=-DDEBUG
CFLAGS=-Wall -g

.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBUG_FLAG} -c $<

.PHONY: all clean test

all:${TARGET}

${TARGET}:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

debug:
	${MAKE} DEBUG_FLAG="-DDEBUG" OBJS="${OBJS} test.o"

clean:
	@rm -f *.o ${TARGET}
