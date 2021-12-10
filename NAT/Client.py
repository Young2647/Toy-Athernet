import socket
import sys
import os
import time
import random
import string

def Getch():
    def _GetchUnix():
        import sys, tty, termios
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

    def _GetchWindows():
        import msvcrt
        return msvcrt.getch()

    try:
        impl = _GetchWindows()
    except ImportError:
        impl = _GetchUnix()
    return impl


def randomString(n):
    payload = ''.join(random.choices(string.ascii_uppercase + string.digits, k=20 )) 
    return bytes(payload, encoding='utf8')


class Client :
    def __init__(self, ip, port, icmp_on = False) :
        self.icmp_on = icmp_on
        self.ip_port = (ip, port)
        if icmp_on:
            self.clientsocket = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_ICMP)
        else:
            self.clientsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


    def sendData(self, data) :
        self.clientsocket.sendto(data, self.ip_port)

    def StopClient(self) :
        self.sendData("Exit".encode())
        print("Client has stopped.")

    def __del__(self) :
        self.clientsocket.close()

if __name__ == "__main__" :
    client = Client("10.20.225.5", 23333)
    while True :
        if Getch() :
            break
        t = time.time()
        data = randomString(20)
        print (data.decode())
        client.sendData(data)
        time.sleep(1)
    client.StopClient()