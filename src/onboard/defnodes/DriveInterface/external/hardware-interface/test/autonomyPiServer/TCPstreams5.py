#!/usr/bin/env python3
import socket
import time

defaultport = 5000

class clientCon:
  def __init__(self,Host,Port):
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((Host, Port))
    self.conn = conn

    CurrentUTC_time = time.time()
    self.info = {
      'TotalSent':0,
      'TotalRecv':0,
      'InitTime':CurrentUTC_time,
      'LastPacket':CurrentUTC_time,
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

  def sendstdat(self,strdat):
    try:
      self.info['TotalSent'] += len(strdat)
      self.conn.send(bytes(strdat,'utf-8'))
      return True
    except socket.error:
      self.close()
      return False

  def getdat(self,buf=1024):
    GOT = self.conn.recv(buf)
    if GOT == b'':
      self.close()
      return False

    self.info['TotalRecv'] += len(GOT)
    self.info['LastPacket'] = time.time()
    return GOT

  def getstdat(self,buf=1024):
    GOT = self.getdat(buf)
    if GOT != False:
      return GOT.decode('utf-8')
    else:
      return GOT
    
  def close(self):
    self.conn.close()
    self.info['Alive'] = False

  def isAlive(self):
    return self.info['Alive']

  def report(self):
    return self.info

  def __del__(self):
    self.close()
    del self.info



def newServer(Host,Port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    s.bind((Host, Port))
    s.listen(1)
    return s
    
    
class serverCon:
    def __init__(self,Server):
      conn, addr = Server.accept()
      self.conn = conn
      CurrentUTC_time = time.time()

      self.info = {
        'Address':{'IP':addr[0],'Port':addr[1]},
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

    def sendstdat(self,strdat):
      try:
        self.info['TotalSent'] += len(strdat)
        self.conn.send(bytes(strdat,'utf-8'))
        return True
      except socket.error:
        self.close()
        return False

    def getdat(self,buf=1024):
      GOT = self.conn.recv(buf)
      if GOT == b'':
        self.close()
        return False

      self.info['TotalRecv'] += len(GOT)
      self.info['LastPacket'] = time.time()
      return GOT

    def getstdat(self,buf=1024):
      GOT = self.getdat(buf)
      if GOT != False:
        return GOT.decode('utf-8')
      else:
        return GOT
      
    def close(self):
      self.conn.close()
      self.info['Alive'] = False

    def isAlive(self):
      return self.info['Alive']

    def report(self):
      return self.info

    def __del__(self):
      self.close()
      del self.info
    
  
