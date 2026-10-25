#!/usr/bin/env python3
import socket
import os
import time
from ._GenericNetWrapper import serverCon as genServerCon, clientCon as genClientCon

class clientUnixCon(genClientCon):
  def __init__(self,FILE):
    conn = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    conn.connect(FILE)
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

  def getdat(self,buf=1024):
    try:
      GOT = self.conn.recv(buf)
      if GOT == b'':
        self.close()
        return False
    except OSError:
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
    self.close()
    del self.info


def newUnixServer(FILE):
  if os.path.exists(FILE):
    os.remove(FILE)
  s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
  s.bind(FILE)

  s.listen(1)
  return s  
    
    
class serverUnixCon(genServerCon):
      def __init__(self,Server):
        conn, addr = Server.accept()
        self.conn = conn
        CurrentUTC_time = time.time()

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
        GOT = self.conn.recv(buf)
        if GOT == b'':
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
        self.close()
        del self.info
    
  
