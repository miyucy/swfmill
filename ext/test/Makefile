OBJS = buffer_test.o inflate_test.o deflate_test.o

CXX     = g++
CFLAGS  = -g -O0 -I../ -I$(HOME)/include
LDFLAGS = -L$(HOME)/lib -lgtest_main -lz -lpthread

all: $(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o test_all $(OBJS)
	./test_all || rm ./test_all

.cc.o:
	$(CXX) $(CFLAGS) -c $<

PHONY: check-syntax
check-syntax:
	$(CXX) $(CFLAGS) -Wall -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)
