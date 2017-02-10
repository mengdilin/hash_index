CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -Wall
LDFLAGS=-g -Wall

SRCS=test.cpp DataEntry.cpp Page.cpp HashIndex.cpp
OBJS=$(subst .cc,.o,$(SRCS))

.PHONY:
all: test

test: $(OBJS)
	$(CXX) $(LDFLAGS) -o test $(OBJS) $(LDLIBS)

