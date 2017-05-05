CC=gcc
CXX=g++
RM=rm -f
LDFLAGS=-std=c++11 -O4 -g
HASH_SIZE ?= 4096
SRCS=test.cpp DataEntry.cpp DataEntry.h Page.cpp Page.h HashIndex.cpp HashIndex.h
OBJS=$(subst .cc,.o,$(SRCS))

.PHONY:
all: test

test: $(OBJS)
	$(CXX) $(LDFLAGS) -o $(HASH_SIZE)_hash_test $(OBJS) $(LDLIBS)

