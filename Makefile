CC=gcc
OBJS=main.o netutil.o log.o iftap.o table.o net.o vxlan.o #test.o mpool.o 
SRCS=${OBJS:%.o=%.c}
LDLIBS=-lpthread
TARGET=vxland
DEBUG_FLAG=-DDEBUG
CFLAGS=-Wall -g

TEST=test


.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBUG_FLAG} -c $<

.PHONY: all clean debug

all:${TARGET}

${TARGET}:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

#debug:
#	${MAKE} "CFLAGS=${CFLAGS}" ${DEBUG_FLAG} "OBJS=${OBJS} test.o"

${TEST}:${OBJS}
	${CC} ${CFLAGS} ${DEBUG_FLAG} ${LDFLAGS} -c test.c
	${CC} ${CFLAGS} ${DEBUG_FLAG} ${LDFLAGS} -o $@ $^ test.o ${LDLIBS}

clean:
	@rm -f *.o ${TARGET}
