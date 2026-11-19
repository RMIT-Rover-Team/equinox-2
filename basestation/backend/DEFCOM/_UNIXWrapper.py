#!/usr/bin/env python3
import socket
import os
import time
from ._GenericNetWrapper import genericNetWrapper

class clientUnixCon(genericNetWrapper):
  def __init__(self,FILE):
    conn = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    conn.connect(FILE)

    super().__init__(conn)


def newUnixServer(FILE):
  if os.path.exists(FILE):
    os.remove(FILE)
  s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
  s.bind(FILE)

  s.listen(1)
  return s  
    
    
class serverUnixCon(genericNetWrapper):
      def __init__(self,Server):
        conn, addr = Server.accept()

        super().__init__(conn, addr)
