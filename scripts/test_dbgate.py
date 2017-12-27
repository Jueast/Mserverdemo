import mpack_pb2
import socket
import hashlib
import time
def controlMpack():
    m = mpack_pb2.Mpack()
    m.type = mpack_pb2.Mpack.CONTROL
    return m
def sendtest(m):
    message = m.SerializeToString();
    print("==============================")
    print("sending {}".format(str(m)))
    sent = udpsocket.sendto(message, server_address)
    time.sleep(1)
    print("waitting to receive")
    data, server = udpsocket.recvfrom(8096)
    m2 = mpack_pb2.Mpack()
    m2.ParseFromString(data)
    print("Result is {}".format(str(m2)))
    return m2

if __name__ == '__main__':
    udpsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_address = ('localhost', 9876)


try:
   m = controlMpack()
   m.control = mpack_pb2.Mpack.MOUNT_WORLD
   sendtest(m)

   m = controlMpack()
   m.control = mpack_pb2.Mpack.SYNC
   m.world.attrs[10045].name = "test"
   m.world.attrs[10045].value = 12345
   m.players[2].attrs[10001].name = "qq"
   m.players[2].attrs[10001].value = 10001
   sendtest(m)

   m = controlMpack()
   m.control = mpack_pb2.Mpack.CREATE_USER
   m.login.uid = 2
   m.login.username = "xiaotiancai"
   m.login.salt = hashlib.md5("123456".encode('utf-8')).hexdigest()
   sendtest(m)

   m = controlMpack()
   m.control = mpack_pb2.Mpack.MOUNT_USER
   m.login.uid = 2
   sendtest(m)

finally:
    print('closing socket')
