BIN = append
VERSION = v0.6

SRC =	main.c

DESTDIR = /usr/local/bin

CPPFLAGS = -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\" -DNDEBUG

DEBUG = -g -O0 -DDEBUG
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -Os

LDFLAGS =

.PREFIX: .c.o

all:
	${CC} -o ${BIN} ${CPPFLAGS} ${CFLAGS} ${SRC} ${LDFLAGS}

install: all
	install -sp -o root -g root --mode=755 -t ${DESTDIR} ${BIN}

## Build with debugging flags when told to produce .o files.
OBJS = ${SRC:.c=.o}
.c.o:
	${CC} -c $< -o $@ ${CPPFLAGS} ${CFLAGS} ${DEBUG}

debug: ${OBJS}
	${CC} ${OBJS} ${LDFLAGS} -o ${BIN}-$@

clean:
	-rm -f ${BIN} ${BIN}-debug ${OBJS}

.PHONY: clean debug install all
