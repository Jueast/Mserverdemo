import mpack_pb2
import socket
import hashlib

def buildLoginMpack(uid, username, salt):
    m = mpack_pb2.Mpack()
    m.type = mpack_pb2.Mpack.LOGIN
    m.login.uid = uid
    m.login.username = username
    m.login.salt = hashlib.md5(salt.encode('utf-8')).hexdigest()
    return m

if __name__ == '__main__':
    udpsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = ('localhost', 9876)
    

try:
    
    testcases = [
        (1, "test", "123456"),
        (2, "test", "123456"),
        (1, "test2", "123456"),
        (2, "test2", "123456"),
        (1, "test", "1234"),
        (2, "test", "1234"),
        (1, "test2", "1234"),
        (2, "test2", "1234")
    ]
    def sendtest(m):
        message = m.SerializeToString();
        print("sending {}".format(str(m)))
        sent = udpsocket.sendto(message, server_address)
        print("waitting to receive")
        data, server = udpsocket.recvfrom(8096)
        m2 = mpack_pb2.Mpack()
        m2.ParseFromString(data)
        print("Result is {}".format(str(m2)))

    for t in testcases:
        m = buildLoginMpack(*t);
        sendtest(m);

finally:
    print('closing socket')
    udpsocket.close()
