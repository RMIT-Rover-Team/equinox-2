#!/usr/bin/env python3
import socket
from ._GenericNetWrapper import genericNetWrapper

class clientTCPCon(genericNetWrapper):
  def __init__(self,Host,Port):
    conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    conn.connect((Host, Port))

    super().__init__(conn)



def newTCPServer(Host,Port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    s.bind((Host, Port))
    s.listen(1)
    return s
    
    
class serverTCPCon(genericNetWrapper):
    def __init__(self,Server):
      conn, addr = Server.accept()

      super().__init__(conn,addr)
      
      
