from os import urandom
import socket
import sys

serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

host = socket.gethostname()

port = 23333

serversocket.bind((host, port))

serversocket.listen(5)

serversocket.settimeout(5)
clientsocket, addr = serversocket.accept()

print("address is : " + str(addr))

msg = "server test sting.\n"

clientsocket.send(msg.encode('utf-8'))
msg = clientsocket.recv(1024)

print(msg.decode('utf-8'))
clientsocket.close()
serversocket.close()
