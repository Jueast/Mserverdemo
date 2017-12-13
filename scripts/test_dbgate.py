import mpack_pb2
import socket
import hashlib
import time
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
        ((1, "admin", "123456"), False),
        ((2, "admin", "123456"), False),
        ((1, "test2", "123456"), False),
        ((2, "test2", "123456"), True),
        ((1, "admin", "1234"), True),
        ((2, "admin", "1234"), True),
        ((1, "test2", "1234"), True),
        ((2, "test2", "1234"), True)
    ]
    def sendtest(m, result):
        message = m.SerializeToString();
        print("==============================")
        print("sending {}".format(str(m)))
        sent = udpsocket.sendto(message, server_address)
        time.sleep(2)
        print("waitting to receive")
        data, server = udpsocket.recvfrom(8096)
        m2 = mpack_pb2.Mpack()
        m2.ParseFromString(data)
        print("Result is {}".format(str(m2)))
        print("Desired result is error: {}".format(str(result)))

    for t in testcases:
        m = buildLoginMpack(*t[0]);
        sendtest(m, t[1]);

finally:
    print('closing socket')
    udpsocket.close()
