import socket
from typing_extensions import final
from Client import Client
from Server import Server
import os
import msvcrt
import time
import struct
import random
import select

ICMP_ECHO_REQUEST = 8
DEFALT_TIMEOUT = 1
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

    def receiveFromNode3(self) :
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
    
    def checkNotify(self) :
        if os.path.exists("WRITE_DOWN.txt") :
            os.remove("WRITE_DOWN.txt")

    def notifyAther(self) :
        f = open ("WRITE_DOWN.txt","w")
        if (self.debug_on) :
            print("Notify the AtherNode to work.")
        f.close()
        time.sleep(0.04)
        os.remove("WRITE_DOWN.txt")

    def ICMPecho(self) :
        while True :
            if msvcrt.kbhit() : break
            if os.path.exists("ICMP_NOTIFY.txt") :
                os.remove("ICMP_NOTIFY.txt")
                break
        if os.path.exists("OUTPUT.bin") :
            with open("OUTPUT.bin","rb") as f :
                ping_address = f.read(4)
                data = f.read()
            ping_address = socket.inet_ntoa(ping_address) #bytes to address
            if (self.debug_on) :
                print("request ping from :", ping_address)    
            packet = self.generateICMPpacket(ping_address,data)
            self.ping_socket = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_ICMP) #raw socket
            self.ping_socket.sendto(packet, (ping_address, 2333))
            self.req_time = time.time()
            reply_time = self.getReplypacket(data)
            if reply_time == -1 :
                print("ICMP ECHO failed!")
            else :
                print("ICMP reply in ", reply_time, " s")
                with open("reply.txt", 'wb') as f:
                    f.write(ping_address)
                    f.write(data)
                self.notifyAther() #notify atherNode to work

    def getReplypacket(self,data) :
        time_remain = DEFALT_TIMEOUT
        final_time = -1
        while True :
            start_time = time.time()
            result = select.select([self.ping_socket], [], [], time_remain)
            if result[0] == [] : break
            end_time = time.time()
            reply_packet, address = self.ping_socket.recvfrom(1024)
            header = reply_packet[20:28]
            icmp_type, code, check_sum, id, sequence = struct.unpack('bbHHh', reply_packet)
            dummy_header = struct.pack('bbHHh', icmp_type, code, 0, id, sequence)
            if id == self.ping_id and check_sum == self.calculateChecksum(dummy_header + data) :
                final_time = end_time - self.req_time
                break
            else :
                time_remain -= end_time - start_time
            if time_remain <= 0 : break
        return final_time
    def calculateChecksum(self, data) :
        s = 1 #waiting to be implemented

    def generateICMPpacket(self,ping_address,data) :
        self.ping_id = random(range(0,1000))
        header = struct.pack('bbHHh', ICMP_ECHO_REQUEST, 0, 0, self.ping_id, 1) #dummy header without checksum
        check_sum = self.calculateChecksum(header + data)
        header = struct.pack('bbHHh', ICMP_ECHO_REQUEST, 0, check_sum, id, 1) # real header
        return header + data
SEND = 0
RECEIVE = 1
ICMP = 2

if __name__ == "__main__" :
    key = input("If send to node3, press s. If receive from node3, press r. If ICMP, press p\n")
    if key == "r" :
        mode = RECEIVE
    elif key == "s":
        mode = SEND
    elif key == "p":
        mode = ICMP
    if mode == SEND :
        node2 = Node2(True,"10.20.220.107", isClient=True)
        node2.receiveFromNode1()
        node2.sendToNode3()
    elif mode == RECEIVE :
        node2 = Node2(True,"10.20.220.107", isClient=False)
        node2.checkNotify()
        node2.receiveFromNode3()
        node2.notifyAther()
    elif mode == ICMP :
        node2 = Node2(True, isClient=True)
        node2.checkNotify()
        node2.ICMPecho()