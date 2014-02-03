CC=gcc
OBJ1=udpgen.o
OBJ2=rawgen.o
OBJ3=arpgen.o
#SRCS1=${OBJ1:%.o=%.c}
#SRCS2=${OBJ2:%.o=%.c}
TARGET1=udpgen
TARGET2=rawgen
TARGET3=arpgen
TARGETS=${TARGET1} ${TARGET2} ${TARGET3}
LDFLAGS=

.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBUG_FLAG} -c $<

.PHONY: all clean

all:${TARGETS}

${TARGET1}:${OBJ1}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

${TARGET2}:${OBJ2}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

${TARGET3}:${OBJ3}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

clean:
	@rm -f *.o ${TARGETS}
