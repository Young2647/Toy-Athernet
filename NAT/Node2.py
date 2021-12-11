import socket
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
    
    def receiveFromNode1(self) :
        while True :
            if msvcrt.kbhit() : break
            if os.path.exists("NOTIFY_DONE.txt") :
                os.remove("NOTIFY_DONE.txt")
                break
        if os.path.exists("output.txt") :
            with open("output.txt") as f :
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
            try :
                os.remove("WRITE_DOWN.txt")
            except :
                pass
    
    def notifyAther(self) :
        f = open ("WRITE_DOWN.txt","w")
        if (self.debug_on) :
            print("Notify the AtherNode to work.")
        f.close()
        time.sleep(0.1)
        try :
            os.remove("WRITE_DOWN.txt")
        except : pass

    def ICMPecho(self) :
        while True :
            if msvcrt.kbhit() : break
            if os.path.exists("ICMP_NOTIFY.txt") :
                os.remove("ICMP_NOTIFY.txt")
                break
        if os.path.exists("request.bin") :
            with open("request.bin","rb") as f :
                request_id = f.read(1)
                ping_address = f.read(4)
                data = f.read()
            ping_address = socket.inet_ntoa(ping_address) #bytes to address
            if (self.debug_on) :
                print("request ping for :", ping_address)    
            packet = self.generateICMPpacket(data)
            icmp = socket.getprotobyname('icmp')
            ping_socket = socket.socket(socket.AF_INET, socket.SOCK_RAW, icmp) #raw socket
            ping_socket.sendto(packet, (ping_address, 80))
            self.req_time = time.time()
            reply_time = self.getReplypacket(ping_socket,data)
            #reply_time = self.receive_ping(ping_socket, 11, 1)
            if reply_time == -1 :
                print("ICMP ECHO failed!")
            else :
                print("ICMP reply in ", reply_time, " s")
                with open("reply.txt", 'wb') as f:
                    f.write(request_id)
                    f.write(socket.inet_aton(ping_address))
                    f.write(data)
                self.notifyAther() #notify atherNode to work
            ping_socket.close()

    def receive_ping(self,my_socket, ID, timeout):
        """
        receive the ping from the socket
        """
        start_time = timeout
        while True:
            start_select = time.perf_counter()
            # select.select(rlist, wlist, xlist[, timeout])
            # wait until ready for read / write / exceptional condition
            # The return value is a triple of lists
            what_ready = select.select([my_socket], [], [], start_time)
            how_long = (time.perf_counter() - start_select)
            if what_ready[0] == []: #timeout
                return
    
    
    def getReplypacket(self, socket, data) :
        time_remain = DEFALT_TIMEOUT
        final_time = -1
        while True :
            start_time = time.time()
            result = select.select([socket], [], [], time_remain)
            if result[0] == [] : break
            end_time = time.time()
            reply_packet, address = socket.recvfrom(1024)
            icmp_header = reply_packet[20:28]
            icmp_type, code, check_sum, id, sequence = struct.unpack('bbHHh', icmp_header)
            dummy_header = struct.pack('bbHHh', icmp_type, code, 0, id, sequence)
            if id == self.ping_id :
                final_time = end_time - self.req_time
                break
            else :
                time_remain -= end_time - start_time
            if time_remain <= 0 : break
        return final_time

    def calculateChecksum(self, data) :
        checksum = 0
        count = (len(data) // 2) * 2
        i = 0

        while i < count:
            temp = data[i + 1] * 256 + data[i]
            checksum = checksum + temp
            checksum = checksum & 0xffffffff 
            i = i + 2

        if i < len(data):
            checksum = checksum + data[len(data) - 1]
            checksum = checksum & 0xffffffff

        # 32-bit to 16-bit
        checksum = (checksum >> 16) + (checksum & 0xffff)
        checksum = checksum + (checksum >> 16)
        checksum = ~checksum
        checksum = checksum & 0xffff

        checksum = checksum >> 8 | (checksum << 8 & 0xff00)
        return checksum



    def generateICMPpacket(self, data) :
        self.ping_id = os.getpid() & 0xFFFF
        header = struct.pack('bbHHH', ICMP_ECHO_REQUEST, 0, 0, self.ping_id, 1) #dummy header without checksum
        check_sum = self.calculateChecksum(header + data)
        header = struct.pack('bbHHH', ICMP_ECHO_REQUEST, 0, socket.htons(check_sum), self.ping_id, 1) # real header
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
        node2 = Node2(True,"10.20.198.211", isClient=True)
        data_sent = 0
        while data_sent < 30 :
            if msvcrt.kbhit() : break
            node2.receiveFromNode1()
            node2.sendToNode3()
            data_sent += 1
        print("All data sent.")
        node2.client.StopClient()
    elif mode == RECEIVE :
        node2 = Node2(True,"10.20.220.107", isClient=False)
        node2.checkNotify()
        node2.receiveFromNode3()
        node2.notifyAther()
    elif mode == ICMP :
        node2 = Node2(True, isClient=True)
        while True :
            if msvcrt.kbhit() : break
            node2.checkNotify()
            node2.ICMPecho()