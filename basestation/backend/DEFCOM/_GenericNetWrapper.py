import time
import socket
import struct

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


class genericNetWrapper:
  def __init__(self, conn, addr=None):
      self.conn = conn
  
      CurrentUTC_time = time.time()

      if type(addr) != tuple:
        addr = ('', 0)

      self.info = {
        'Address':addr,
        'InitTime':CurrentUTC_time,
        'LastPacket':CurrentUTC_time,
        'TotalSent':0,
        'TotalRecv':0,
        'Alive':True
      }


  def senddat(self,bindat):
    try:
      self.info['TotalSent'] += len(bindat)
      self.conn.send(bindat)
      return True
    except socket.error:
      self.close()
      return False

  def getdat(self,buf=1024):
    try:
      GOT = self.conn.recv(buf)
      if GOT == b'':
        self.close()
        return False
    except socket.error:
      self.close()
      return False

    self.info['TotalRecv'] += len(GOT)
    self.info['LastPacket'] = time.time()
    return GOT

    
  def close(self):
    self.conn.close()
    self.info['Alive'] = False

  def isAlive(self):
    return self.info['Alive']

  def report(self):
    return self.info

  def __del__(self):
    try:
      self.close()
      del self.info
    except:
      pass

  def setPriority(self, priority: int):
      if priority > 7:
          raise ValueError("Priority must be between 0 and 7")
      
      self.SOCK.setsockopt(
          socket.IPPROTO_IP,
          socket.IP_TOS,
          struct.pack('B', priMap[priority])
      )