import socket
from Client import Client
from Server import Server
import os
import msvcrt
import time

class Node2 :
    def __init__(self, debug_on = False, ip = "10.20.225.5", port = 23333, isClient = False):
        if isClient :
            self.client = Client(ip, port)  #client sending data to Node 3, so Node 3 ip and port needed here
        else :
            self.server = Server(port) # server receiving data from Node 3
        self.node1data = []
        self.debug_on = debug_on

    def decodeByte(self, data) :
        for line_data in data :
            self.node1data.append(line_data.encode())
    
    def sendToNode3(self) :
        for line_data in self.node1data :
            if self.debug_on :
                print(line_data.decode())
            self.client.sendData(line_data)
            time.sleep(0.04)
        if self.debug_on :
            print("All data sent.")
        self.client.StopClient()
    
    def receiveFromNode1(self) :
        while True :
            if msvcrt.kbhit() : break
            if os.path.exists("NOTIFY_DONE.txt") :
                os.remove("NOTIFY_DONE.txt")
                break
        if os.path.exists("OUTPUT.bin") :
            with open("OUTPUT.bin") as f :
                self.decodeByte(f)

    def checkNotify(self) :
        if os.path.exists("WRITE_DOWN.txt") :
            os.remove("WRITE_DOWN.txt")

    def receiveFromNode3(self) :
        self.checkNotify()
        with open("input.bin", "wb") as inputfile : 
            while True :
                if msvcrt.kbhit() : break
                data, address = self.server.receiveData()
                if data == "Exit" :
                    break
                else :
                    if (self.debug_on) :
                        print("address is : ", address, "data is : ", data)
                    self.writeToFile(inputfile, data.encode('utf8'), address)
            if (self.debug_on) :
                print("All data received.")
            self.server.stopServer()
    
    def writeToFile(self, inputfile, data, address) :
        inputfile.write(socket.inet_aton(address[0]))
        inputfile.write(address[1].to_bytes(2, byteorder = 'little'))
        inputfile.write(data)
    
    def notifyAther(self) :
        f = open ("WRITE_DOWN.txt","w")
        if (self.debug_on) :
            print("Notify the AtherNode to work.")
        f.close()

SEND = 0
RECEIVE = 1

if __name__ == "__main__" :
    if input("If send to node3, press s. If receive from node3, press r\n") == "r" :
        mode = RECEIVE
    else :
        mode = SEND
    if mode == SEND :
        node2 = Node2(True,"10.20.220.107", isClient=True)
        node2.receiveFromNode1()
        node2.sendToNode3()
    elif mode == RECEIVE :
        node2 = Node2(True,"10.20.220.107", isClient=False)
        node2.receiveFromNode3()
        node2.notifyAther()