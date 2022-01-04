import pyshark
import socket
import struct
import logging.config
import sys
import binascii


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

cap = pyshark.LiveCapture('WLAN', bpf_filter='icmp', use_json=True, include_raw=True)
while(True):
    for pkt in cap.sniff_continuously(packet_count=1):
        ip_payload = pkt.get_raw_packet()[34:]
        ip_addr = str(pkt.ip.src)
        dst_ip = str(ip_addr)
        send_ip_packet(dst_ip, ip_payload)
        print('sent')


