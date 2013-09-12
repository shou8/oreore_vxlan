CC=gcc
OBJS=main.o netutil.o log.o iftap.o mpool.o table.o vxlan.o
SRCS=${OBJS:%.o=%.c}
CFLAGS=-Wall
#LDLIBS=-lpthread
TARGET=vxland



.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} ${LDFLAGS} -c $<

.PHONY: all clean debug

all:${TARGET}

${TARGET}:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

debug:
	${MAKE} "CFLAGS=${CFLAGS} -DDEBUG" "OBJS=${OBJS} test.o"

test:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -c test.c
	${CC} ${CFLAGS} -DDEBUG ${LDFLAGS} -o $@ $^ test.o ${LDLIBS}

clean:
	@rm -f *.o ${TARGET}
