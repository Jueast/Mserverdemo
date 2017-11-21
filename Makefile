CC=g++
STD=c++11
CFLAGS=-lpthread -lboost_system -std=$(STD)

BDIR=bin

%.o: %.cpp
	$(CC) -o $@ -c $^ $(CFLAGS)
server: server.o
	$(CC) -o ${BDIR}/$@ $^ $(CFLAGS) 

client: client.o
	$(CC) -o ${BDIR}/$@ $^ $(CFLAGS)

clean:
	rm -f *.o *~ server client 
