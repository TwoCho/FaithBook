# Set compiler to use
CC=g++ -std=c++11
CFLAGS=-g -I. -fpermissive
LDFLAGS=-lpthread
DEBUG=0

all:: ChatServ.exe 

ChatServ.exe: ChatServ.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

%.o: %.c $(DEPS_CHAT)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(DEPS_TET)
	$(CC) -c -o $@ $< $(CFLAGS)

clean: 
	rm -f *.exe *.o *~ *.stackdump
