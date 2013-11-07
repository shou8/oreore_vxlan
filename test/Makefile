CC=gcc
OBJ1=udpgen.o
OBJ2=rawgen.o
#SRCS1=${OBJ1:%.o=%.c}
#SRCS2=${OBJ2:%.o=%.c}
TARGET1=udpgen
TARGET2=rawgen
TARGETS=${TARGET1} ${TARGET2}
LDFLAGS=

.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} ${LDFLAGS} ${DEBUG_FLAG} -c $<

.PHONY: all clean

all:${TARGET1} ${TARGET2}

${TARGET1}:${OBJ1}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

${TARGET2}:${OBJ2}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ ${LDLIBS}

clean:
	@rm -f *.o ${TARGETS}
