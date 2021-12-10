import socket
from Client import Client
from Client import Getch
from Server import Server
import os



class Node2 :
    def __init__(self, debug_on = False):
        self.client = Client("10.20.225.5", 23333)  #client sending data to Node 3, so Node 3 ip and port needed here
        self.server = Server(23333) # server receiving data from Node 3
        self.node1data = []
        self.debug_on = debug_on

    def decodeByte(self, data) :
        for line_data in data :
            self.node1data.append(bytes(line_data, 'utf-8'))
    
    def sendToNode3(self) :
        for line_data in self.node1data :
            if self.debug_on :
                print(line_data.decode())
            self.client.sendData(line_data)
        if self.debug_on :
            print("All data sent.")
        self.client.StopClient()
    
    def receiveFromNode1(self) :
        while True :
            if Getch() : break
            if os.path.exists("NOTIFY_DONE.txt") :
                os.remove("NOTIFY_DONE.txt")
                break
        if os.path.exists("OUTPUT.bin") :
            with open("OUTPUT.bin") as f :
                data = f.read()
                self.decodeByte(data)

    def receiveFromNode3(self) :
        with open("input.bin", "wb") as inputfile : 
            while True :
                if Getch : break
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
        #inputfile.write(socket.inet_aton(address[0]))
        #inputfile.write(address[1].to_bytes(2, byteorder = 'little'))
        inputfile.write(data)
    
    def notifyAther(self) :
        f = open ("WRITE_DOWN.txt","w")
        if (self.debug_on) :
            print("Notify the AtherNode to work.")
        f.close()

SEND = 0
RECEIVE = 1

if __name__ == "__main__" :
    node2 = Node2(True)
    if input("If send to node3, press s. If receive from node3, press r\n") == "r" :
        mode = RECEIVE
    else :
        mode = SEND
    if mode == SEND :
        node2.receiveFromNode1()
        node2.sendToNode3()
    elif mode == RECEIVE :
        node2.receiveFromNode3()
        node2.notifyAther()
