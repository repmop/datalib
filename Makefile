CC = g++
CFLAGS = -g -gstabs -ggdb -Wall -Wextra -Werror -std=gnu++11

all: harness

q.o: q.cpp q.h
	$(CC) $(CFLAGS) -c q.cpp

harness.o: harness.cpp
	$(CC) $(CFLAGS) -c harness.cpp

skip.o: skip.cpp
	$(CC) $(CFLAGS) -c skip.cpp

harness: q.o skip.o harness.o

clean:
	rm -f *~ *.o *.tar *.zip *.gzip *.bzip *.gz