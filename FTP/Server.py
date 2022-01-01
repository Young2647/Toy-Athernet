import socket
import sys
import os

class Server :
    def __init__(self, port):
        self.serversocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.serversocket.bind(('', port))

    def receiveData(self) :
        # self.serversocket.settimeout(5)
        data, address = self.serversocket.recvfrom(1024)
        data = data.decode('utf8')
        return data, address

    def stopServer(self) :
        print("Server has stopped.")
    
    def __del__(self) :
        self.serversocket.close()

if __name__ == "__main__" :
    server = Server(23333)
    while True :
        data, address = server.receiveData()
        print("address is : ", address, "data is : ", data)
        if data == "Exit" :
            break
    server.stopServer()    