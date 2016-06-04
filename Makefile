EXE_NAME='tst'

CC=gcc
CXX=g++

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

CXXFLAGS=-std=c++11
CXXFLAGS+=-g

all:
	${CC} ${CFLAGS} -c event-msgpack.c
	${CXX} ${CXXFLAGS} -I. -c ./test/readTest.cpp
	${CXX} ${CXXFLAGS} -I. -c ./test/writeTest.cpp
	${CXX} ${CXXFLAGS} -I. -o ${EXE_NAME} event-msgpack.o ./test/test.cpp readTest.o writeTest.o -lcppunit
