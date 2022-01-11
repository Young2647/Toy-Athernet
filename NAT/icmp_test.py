import pyshark
import socket
import struct
import logging.config
import sys
import binascii
import os
import msvcrt
from scapy.all import *
from time import sleep

def calculateChecksum(data) :
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





def send_ip_packet(dst_ip, icmp_raw):
    icmp = socket.getprotobyname('icmp')
    ping_reply = socket.socket(socket.AF_INET, socket.SOCK_RAW, icmp)
    icmp_raw = bytearray(icmp_raw)
    icmp_raw[0:1] = b'\x00'
    icmp_raw[2:4] = b'\x00\x00'
    checksum = int(calculateChecksum(icmp_raw)).to_bytes(length=2, byteorder='big')
    icmp_raw[2:4] = checksum
    ping_reply.sendto(bytes(icmp_raw), (dst_ip, 80))


def notifyAther() :
    f = open ("WRITE_DOWN.txt","w")
    print("Notify the AtherNode to work.")
    f.close()

def sendIptoNode1(ip_addr) :
    with open("reply.txt", 'wb') as f:
        f.write(ip_addr)
    notifyAther()

def waitNode1apply() :
    while True :
        if msvcrt.kbhit() : break
        if os.path.exists("NOTIFY_DONE.txt") :
            os.remove("NOTIFY_DONE.txt")
            break
    if os.path.exists("output.txt") :
        with open("output.txt") as f :
            send_ip_packet(f.read())

    
def pack_callback(pkt):
    # print(pkt.show())
    ip_payload = bytes(pkt['IP'].payload)
    ip_addr = str(pkt['IP'].src)
    sendIptoNode1(socket.inet_aton(ip_addr))
    waitNode1apply()
    send_ip_packet(ip_addr, ip_payload)
    print('sent')

if __name__ == "__main__":
    sniff(filter = 'icmp', prn=pack_callback, iface = 'WLAN 2', count=0)
   



   
    # cap = pyshark.LiveCapture('WLAN', bpf_filter='icmp', use_json=True, include_raw=True)
    # while(True):
    #     for pkt in cap.sniff_continuously(packet_count=1):
    #         ip_payload = pkt.get_raw_packet()[34:]
    #         print(ip_payload[15])
    #         print(ip_payload[16] == 255)
    #         print(ip_payload[17] == 255)
    #         ip_addr = str(pkt.ip.src)
    #         sendIptoNode1(ip_addr)
    #         waitNode1apply()
    #         #send_ip_packet(ip_addr, ip_payload)
    #         print('sent')


