CC=g++
STD=c++11
CFLAGS=-lpthread -lboost_system -std=$(STD)
SRC_DIR=src
BIN_DIR=bin
PROTO_DIR=proto
PROTO_NAME=request
TEMP_DIR=tmp
%.o: ${SRC_DIR}/%.cpp
	$(CC) -o $@ -c $^ $(CFLAGS)
server: server.o
	$(CC) -o ${BIN_DIR}/$@ $^ $(CFLAGS) 

client: client.o
	$(CC) -o ${BIN_DIR}/$@ $^ $(CFLAGS)

protobuf: ${PROTO_DIR}/${PROTO_NAME}.proto
	mkdir -p ${TEMP_DIR}
	protoc -I=${PROTO_DIR} --cpp_out=${TEMP_DIR} ${PROTO_DIR}/${PROTO_NAME}.proto


clean:
	rm -rf *.o *~ server client tmp
