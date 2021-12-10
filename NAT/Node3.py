import socket
from Client import Client
from Client import Getch
from Server import Server
import os

class Node3 :
    def __init__(self, debug_on = False):
        self.client = Client("10.20.220.107", 23333) # send data to node2, node2 address needed 
        self.server = Server(23333)
        self.debug_on = debug_on
    def receiveFromNode2(self) :
        with open("output.bin", "w") as outputfile : 
            while True :
                if Getch() : break
                data, address = self.server.receiveData()
                if data == "Exit" :
                    break
                else :
                    if (self.debug_on) :
                        print("address is : ", address, "data is : ", data)
                        self.writeToFile(outputfile, data, address)
            if (self.debug_on) :
                print("All data received.")
            self.server.stopServer()
    
    def sendToNode2(self) :
        if os.path.exists("input.txt") :
            with open("input.txt") as inputfile :
                for line_data in inputfile :
                    if self.debug_on :
                        print(line_data)
                    self.client.sendData(line_data.encode('utf8'))
                if self.debug_on :
                    print("All Data Sent.")
            self.client.StopClient()
    
    def writeToFile(self, outputfile, data, address) :
        outputfile.write(data)

SEND = 0
RECEIVE = 1

if __name__ == "__main__" :
    node3 = Node3(True)
    if input("If send to node2, press s. If receive from node2, press r\n") == "r" :
        mode = RECEIVE
    else :
        mode = SEND
    if mode == SEND :
        node3.sendToNode2()
    elif mode == RECEIVE :
        node3.receiveFromNode2()