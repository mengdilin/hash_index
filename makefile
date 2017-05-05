CC=gcc
CXX=g++
RM=rm -f
LDFLAGS=-std=c++11 -O3 -g

SRCS=test.cpp DataEntry.cpp Page.cpp HashIndex.cpp
OBJS=$(subst .cc,.o,$(SRCS))

.PHONY:
all: test

test: $(OBJS)
	$(CXX) $(LDFLAGS) -o test $(OBJS) $(LDLIBS)

