# An example process
from AbstractProcess import AbstractProcess
import time

"""
Equinox 2 Monothread application V1.0

Authors:
- Kaelan Grainger (MegaKG)

"""

class DummyProcess(AbstractProcess):
    def __init__(self, a=1) -> None:
        self.a = a
        
        
    def run(self) -> None:
        while True:
            self.print("A is: ",self.a)
            time.sleep(1)
