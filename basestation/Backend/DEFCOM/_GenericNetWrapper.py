class serverCon:
    pass

class clientCon:
  def senddat(self,bindat: bytes) -> bool:
      pass

  def getdat(self,buf: int = 1024) -> bytes:
    pass

  def close(self):
    pass

  def isAlive(self) -> bool:
    pass

  def report(self) -> dict:
    pass

  def __del__(self):
    pass