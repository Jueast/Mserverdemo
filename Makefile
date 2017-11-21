CC=g++
CFLAGS=-lpthread -lboost_system


%.o: %.c
	$(CC) -c -o $@ $(CFLAGS)
server: server.o
	$(CC) -o $@ $^ $(CFLAGS) 

client: client.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o *~ server client 
