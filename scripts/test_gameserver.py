import mpack_pb2
import socket
import hashlib
import struct
import time 

def send_packedMessage(sock, m):
    message = m.SerializeToString()
    sent = sock.send(struct.pack("<I", len(message)));
    sent = sock.send(message)
    return 

def recv_packedMessage(sock):
    chunk = sock.recv(4)
    l = struct.unpack("<I", chunk)[0]
    data = sock.recv(l)
    m2 = mpack_pb2.Mpack()
    m2.ParseFromString(data)
    return m2

def sendtest(m, sock):
    print("===========================")
    print("sending {}".format(str(m)))
    send_packedMessage(sock, m)
    print("waitting to receive")
    m2 = recv_packedMessage(sock)
    print("Result is {}".format(str(m2)))
    return m2 


if __name__ == '__main__':
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
    server_address = ('localhost', 10000)
    sock.connect(server_address)

try: 
    m = mpack_pb2.Mpack()
    m.type = mpack_pb2.Mpack.REGISTER
    m.login.uid = 3
    m.login.username = "redog"
    m.login.salt = hashlib.md5("123456".encode('utf-8')).hexdigest()
    
    r = sendtest(m, sock)
    if(r.error):
        m.type = mpack_pb2.Mpack.LOGIN
        sendtest(m, sock) 

    m = mpack_pb2.Mpack()
    m.type = mpack_pb2.Mpack.STATE_QUERY
    m.world.attrs[10045].name = "ask"
    m.players[3].attrs[10001].name = "test"  
    sendtest(m, sock)

    m = mpack_pb2.Mpack()
    m.type = mpack_pb2.Mpack.STATE_MODIFY
    m.players[3].attrs[10001].name = "qq"
    m.players[3].attrs[10001].value = 10001
    sendtest(m, sock)

finally:
    print("close socket.")
    sock.close()




