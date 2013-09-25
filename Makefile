CC=gcc
OBJS=main.o netutil.o log.o iftap.o table.o net.o vxlan.o #mpool.o 
SRCS=${OBJS:%.o=%.c}
CFLAGS=-Wall
#LDLIBS=-lpthread
TARGET=vxland
DEBUG_FLAG=-DDEBUG



.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBUG_FLAG} -c $<

.PHONY: all clean debug

all:${TARGET}

${TARGET}:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

debug:
	${MAKE} "CFLAGS=${CFLAGS}" ${DEBUG_FLAG} "OBJS=${OBJS} test.o"

test:${OBJS}
	${CC} ${CFLAGS} ${DEBUG_FLAG} ${LDFLAGS} -c test.c
	${CC} ${CFLAGS} ${DEBUG_FLAG} ${LDFLAGS} -o $@ $^ test.o ${LDLIBS}

clean:
	@rm -f *.o ${TARGET}
