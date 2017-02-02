CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-g -Wall
LDFLAGS=-g -Wall

SRCS=test.cpp DataEntry.cpp Page.cpp HashIndex.cpp
OBJS=$(subst .cc,.o,$(SRCS))

all: test

test: $(OBJS)
	$(CXX) $(LDFLAGS) -o test $(OBJS) $(LDLIBS)

test.o: test.cpp Page.h DataEntry.h

DataEntry.o: DataEntry.h

Page.o: Page.h DataEntry.h

HashIndex.o: HashIndex.h Page.h

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) test
