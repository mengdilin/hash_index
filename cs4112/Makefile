C = gcc
CXX = g++

INCLUDES =

# Compilation options:
# -g for debugging info and -Wall enables all warnings
CFLAGS = -g -msse4a -msse4.2 -Wall $(INCLUDES)
CXXFLAGS = -g -msse4a -msse4.2 -Wall $(INCLUDES)

main = build

# Linking options:
LDFLAGS = -g -msse4.2 -Wall $(INCLUDES)

# List the libraries you need to link with in LDLIBS
LDLIBS =

$(main): main.o
	$(CC) $(LDFLAGS) main.o -lm -o $(main)

main.o: main.c

.PHONY: clean
clean:
	rm -f *.o $(main)

