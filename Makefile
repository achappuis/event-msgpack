EXE_NAME='tst'

CC=gcc

CFLAGS=--std=c99 -Werror -Wall -Wextra
CFLAGS+=-funsigned-char
CFLAGS+=-fwrapv
CFLAGS+=-Wmissing-format-attribute
CFLAGS+=-Wpointer-arith
CFLAGS+=-Wformat-nonliteral
CFLAGS+=-Winit-self
CFLAGS+=-Wwrite-strings
CFLAGS+=-Wshadow
CFLAGS+=-Wenum-compare
CFLAGS+=-Wempty-body
#CFLAGS+=-Wsizeof-array-argument" # GCC does not support this
#CFLAGS+=-Wstring-conversion" # GCC does not support this
CFLAGS+=-Wparentheses
CFLAGS+=-Wcast-align
CFLAGS+=-Wstrict-aliasing
CFLAGS+=--pedantic-errors
CFLAGS+=-g

all:
	${CC} ${CFLAGS} -c event-msgpack.c
	${CC} -I. -o ${EXE_NAME} event-msgpack.o ./test/test.c
