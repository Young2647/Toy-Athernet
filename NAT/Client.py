import socket
import sys
import os
import time

def randomString(n):
    return os.urandom(n)

class Client :
    def __init__(self, ip, port) :
        self.ip_port = (ip, port)
        self.clientsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def sendData(self, data) :
        self.clientsocket.sendto(data, self.ip_port)

    def StopClient(self) :
        self.sendData("Exit")
        print("Client has stopped.")
        self.clientsocket.close()

if __name__ == "__main__" :
    client = Client("10.19.73.48", 23333)
    while True :
        if input() :
            break
        t = time.time()
        data = randomString(20)
        print (data.decode())
        client.sendData(data)
        time.sleep(1)
    client.StopClient()