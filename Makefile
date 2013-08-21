CC=gcc
OBJS=netutil.o log.o iftap.o # net.o vxlan.o main.o
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

clean:
	@rm -f *.o ${TARGET}
