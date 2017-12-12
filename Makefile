CC=g++
STD=c++11
INC_DIR=include
PROTOFLAGS=$(pkg-config --cflags --libs protobuf)
LDFLAGS=-lpthread -lboost_system -lprotobuf -lmysqlpp -lmysqlclient
CFLAGS=-Wall -std=$(STD) -I. -I${INC_DIR} -g $(LDFLAGS) -I/usr/include/mysql -L/usr/local/lib -I/usr/local/include
SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=obj
SCRIPTS_DIR=scripts
PROTO_DIR=proto
PROTO_NAME=mpack
TEMP_DIR=prototmp
OBJS=mpack_message.o ${PROTO_NAME}.pb.o logging.o pubixml.o
DEPS=${INC_DIR}/mpack_message.hpp  

all: ${BIN_DIR} protobuf dbgate 
	rm *.o
	mkdir -p ${BIN_DIR}/log

${BIN_DIR}:
	mkdir -p ${BIN_DIR}

${PROTO_NAME}.pb.o: ${SRC_DIR}/${PROTO_NAME}.pb.cc ${INC_DIR}/${PROTO_NAME}.pb.h
	$(CC) -o $@ -c $< $(CFLAGS)


pubixml.o: ./pugixml/src/pugixml.cpp
	$(CC) -o $@ -c $< $(CFLAGS)
%.o: ${SRC_DIR}/%.cpp ${DEPS}
	$(CC) -o $@ -c $< $(CFLAGS)

dbgate: mserver_dbgate.o ${OBJS} 
	$(CC) -o ${BIN_DIR}/$@ $^ $(CFLAGS)

protobuf: ${PROTO_DIR}/${PROTO_NAME}.proto
	mkdir -p ${TEMP_DIR}
	protoc -I=${PROTO_DIR} --cpp_out=${TEMP_DIR} --python_out=${SCRIPTS_DIR} ${PROTO_DIR}/${PROTO_NAME}.proto
	mv ${TEMP_DIR}/${PROTO_NAME}.pb.h ${INC_DIR}/
	mv ${TEMP_DIR}/${PROTO_NAME}.pb.cc ${SRC_DIR}/
	rm -r ${TEMP_DIR}

clean:
	rm -rf *.o *~ ${BIN_DIR}

.PHONY: all clean
