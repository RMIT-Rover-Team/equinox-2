#!/usr/bin/env python3
import socket
import struct

__author__ = 'Kaelan Grainger'
__version__ = '3.1'

priMap = [
    0x00, # PCP 0 - Best Effort
    0x20, # PCP 1
    0x40, # PCP 2
    0x60, # PCP 3
    0x80, # PCP 4
    0xA0, # PCP 5 - Critical
    0xC0, # PCP 6 - Internetwork Control
    0xE0  # PCP 7 - Network Control
]


class udpsend:
    def __init__(self, IP, PORT, BUFF_SIZE=1024, MCAST_GRP="239.0.0.1"):
        """
        IP = interface IP to send from (e.g., 192.168.40.22)
        PORT = destination port
        """
        self.iface_ip = IP
        self.port = PORT
        self.buffs = BUFF_SIZE
        self.mcastgp = MCAST_GRP

        self.SOCK = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.SOCK.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, BUFF_SIZE)

        # Bind socket to interface IP so outgoing multicast uses that NIC
        self.SOCK.bind((self.iface_ip, 0))

        # TTL = 1 (stay on LAN)
        ttl = struct.pack('b', 1)
        self.SOCK.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

        # Set outgoing interface for multicast
        self.SOCK.setsockopt(
            socket.IPPROTO_IP,
            socket.IP_MULTICAST_IF,
            socket.inet_aton(self.iface_ip)
        )

    def setPriority(self, priority: int):
        if priority > 7:
            raise ValueError("Priority must be between 0 and 7")
        
        self.SOCK.setsockopt(
            socket.IPPROTO_IP,
            socket.IP_TOS,
            struct.pack('B', priMap[priority])
        )

    def senddat(self, DATA):
        """Send to hardcoded multicast group."""
        self.SOCK.sendto(DATA, (self.mcastgp, self.port))

    def close(self):
        self.SOCK.close()


class udpget:
    def __init__(self, IP, PORT, BUFF_SIZE=1024, MCAST_GRP="239.0.0.1"):
        """
        IP = interface IP to listen on (e.g., 192.168.40.22)
        PORT = multicast port
        """
        self.iface_ip = IP
        self.port = PORT
        self.buffs = BUFF_SIZE
        self.mcastgp = MCAST_GRP

        self.SOCK = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)

        # Allow multiple listeners
        self.SOCK.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        # Bind to any received IP
        self.SOCK.bind(("0.0.0.0", PORT))

        # Join multicast group only on this interface with the required IP
        mreq = struct.pack(
            "4s4s",
            socket.inet_aton(self.mcastgp),
            socket.inet_aton(self.iface_ip)
        )
        #print(self.iface_ip)
        self.SOCK.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    def getdat(self):
        return self.SOCK.recvfrom(self.buffs)[0]

    def getaddrdat(self):
        return self.SOCK.recvfrom(self.buffs)

    def close(self):
        self.SOCK.close()
