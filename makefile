CC=cc
CXX=g++
RM=rm -f
LDFLAGS=-std=c++11 -O2 -g -Wall

SRCS=test.cpp DataEntry.cpp BTreePage.cpp BTree.cpp
OBJS=$(subst .cc,.o,$(SRCS))

.PHONY:
all: test

test: $(OBJS)
	$(CXX) $(LDFLAGS) -o test $(OBJS) $(LDLIBS)
