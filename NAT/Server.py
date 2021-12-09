import socket
import sys
import os

class Server :
    def __init__(self, port):
        self.serversocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.serversocket.bind(('', port))

    def receiveData(self) :
        data, address = self.serversocket.recvfrom(1024)
        data = data.decode('utf8')
        print("address is : ", address, "data is : ", data)
        return data
    def stopServer(self) :
        print("Server has stopped.")
        self.serversocket.close()

if __name__ == "__main__" :
    server = Server(23333)
    while True :
        data = server.receiveData()
        if data == "Exit" :
            break
    server.stopServer()    