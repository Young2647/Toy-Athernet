import socket
import sys
import os
import time
import msvcrt
import random
import string

def randomString(n):
    payload = ''.join(random.choices(string.ascii_uppercase + string.digits, k=20 )) 
    return bytes(payload, encoding='utf8')


class Client :
    def __init__(self, ip, port) :
        self.ip_port = (ip, port)
        self.clientsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def sendData(self, data) :
        self.clientsocket.sendto(data, self.ip_port)

    def StopClient(self) :
        self.sendData("Exit".encode())
        print("Client has stopped.")
        self.clientsocket.close()

if __name__ == "__main__" :
    client = Client("10.20.225.5", 23333)
    while True :
        if msvcrt.kbhit() :
            break
        t = time.time()
        data = randomString(20)
        print (data.decode())
        client.sendData(data)
        time.sleep(1)
    client.StopClient()