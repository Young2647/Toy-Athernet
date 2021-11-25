import socket
import sys

clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

host = socket.gethostname()

port = 23333

clientsocket.connect((host, port))

msg = clientsocket.recv(1024)

send_msg = "client test string.\n"

clientsocket.send(send_msg.encode('utf-8'))
clientsocket.close()

print(msg.decode('utf-8'))