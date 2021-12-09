import socket
import sys
import os

class Server :
    def __init__(self, ip, port):
        self.ip_port = (ip, port)
        self.serversocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.serversocket.bind(self.ip_port)

    def receiveData(self) :
        data, address = self.serversocket.recvfrom(1024)
        print("address is : {} data is : {}".format(address, data.decode()))
        return data
    def stopServer(self) :
        print("Server has stopped.")
        self.serversocket.close()

if __name__ == "__main__" :
    server = Server("10.19.73.48", 23333)
    while True :
        data = server.receiveData()
        if data == "Exit" :
            break
    server.stopServer()    