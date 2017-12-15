import mpack_pb2
import socket
import hashlib
import struct
import time
def buildLoginMpack(uid, username, salt):
    m = mpack_pb2.Mpack()
    m.type = mpack_pb2.Mpack.LOGIN
    m.login.uid = uid
    m.login.username = username
    m.login.salt = hashlib.md5(salt.encode('utf-8')).hexdigest()
    return m

if __name__ == '__main__':

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
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
            server_address = ('localhost', 10000)
            sock.connect(server_address)
            message = m.SerializeToString();
            print("==============================")
            print("sending {}".format(str(m)))
            sent = sock.send(struct.pack("<I", len(message)))
            sent = sock.send(message)
            time.sleep(1)
            print("waitting to receive")
            chunk = sock.recv(4);
            l = struct.unpack("<I", chunk)[0]
            data = sock.recv(l)
            m2 = mpack_pb2.Mpack()
            m2.ParseFromString(data)
            print("Result is {}".format(str(m2)))
            print("Desired result is error: {}".format(str(result)))
            sock.close()
        for t in testcases:
            m = buildLoginMpack(*t[0]);
            sendtest(m, t[1]);
    finally:
        print("Over.")
