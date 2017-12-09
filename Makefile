CC=g++
STD=c++11
INC_DIR=include
LDFLAGS=-lpthread -lboost_system -lprotobuf -lmysqlpp -lmysqlclient_r
CFLAGS=-std=$(STD) -I. -I${INC_DIR} -g $(LDFLAGS)
SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=obj
PROTO_DIR=proto
PROTO_NAME=mpack
TEMP_DIR=prototmp
OBJS=mpack_message.o ${PROTO_NAME}.pb.o logging.o pubixml.o
DEPS=${INC_DIR}/mpack_message.hpp 

all: ${BIN_DIR} protobuf server client 
	rm *.o

${BIN_DIR}:
	mkdir -p ${BIN_DIR}

${PROTO_NAME}.pb.o: ${SRC_DIR}/${PROTO_NAME}.pb.cc ${INC_DIR}/${PROTO_NAME}.pb.h
	$(CC) -o $@ -c $< $(CFLAGS)


pubixml.o: ./pugixml/src/pugixml.cpp
	$(CC) -o $@ -c $< $(CFLAGS)
%.o: ${SRC_DIR}/%.cpp ${DEPS}
	$(CC) -o $@ -c $< $(CFLAGS)
server: server.o ${OBJS} 
	$(CC) -o ${BIN_DIR}/$@ $^ $(CFLAGS) 

client: client.o ${OBJS} 
	$(CC) -o ${BIN_DIR}/$@ $^ $(CFLAGS)

protobuf: ${PROTO_DIR}/${PROTO_NAME}.proto
	mkdir -p ${TEMP_DIR}
	protoc -I=${PROTO_DIR} --cpp_out=${TEMP_DIR} ${PROTO_DIR}/${PROTO_NAME}.proto
	mv ${TEMP_DIR}/${PROTO_NAME}.pb.h ${INC_DIR}/
	mv ${TEMP_DIR}/${PROTO_NAME}.pb.cc ${SRC_DIR}/
	rm -r ${TEMP_DIR}

clean:
	rm -rf *.o *~ ${BIN_DIR}

.PHONY: all clean
