#!/usr/bin/env python3
import socket
import struct
import netifaces

__author__ = 'Kaelan Grainger'
__version__ = '3.0'


class udpsend:
    def __init__(self, IP=None, PORT=None, BUFF_SIZE=1024):
        self.addr = (IP, PORT)
        self.buffs = BUFF_SIZE

        self.SOCK = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.SOCK.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, BUFF_SIZE)

        # TTL = 1 (stay on LAN)
        ttl = struct.pack('b', 1)
        self.SOCK.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

    def senddat(self, DATA, IP=None, PORT=None):
        if self.addr != (None, None):
            IP, PORT = self.addr
        self.SOCK.sendto(DATA, (IP, PORT))

    def close(self):
        self.SOCK.close()


class udpget:
    def __init__(self, IP, PORT, BUFF_SIZE=1024):
        self.group = IP
        self.port = PORT
        self.buffs = BUFF_SIZE

        self.SOCK = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

        # Allow multiple listeners
        self.SOCK.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        # Bind to port on all interfaces
        self.SOCK.bind(("", PORT))

        # Join multicast group on ALL interfaces
        for iface in netifaces.interfaces():
            addrs = netifaces.ifaddresses(iface).get(netifaces.AF_INET)
            if not addrs:
                continue

            for addr in addrs:
                ip = addr["addr"]
                try:
                    mreq = struct.pack("4s4s",
                        socket.inet_aton(IP),
                        socket.inet_aton(ip)
                    )
                    self.SOCK.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
                    # print(f"Joined {IP} on {ip}")
                except OSError:
                    pass

    def getdat(self):
        return self.SOCK.recvfrom(self.buffs)[0]

    def getaddrdat(self):
        return self.SOCK.recvfrom(self.buffs)

    def close(self):
        self.SOCK.close()
