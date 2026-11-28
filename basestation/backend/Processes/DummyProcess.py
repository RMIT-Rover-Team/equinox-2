# An example process
from AbstractProcess import AbstractProcess
import time


"""
Equinox 2 Monothread application V1.0

Authors:
- Kaelan Grainger (MegaKG)

"""

class DummyProcess(AbstractProcess):
    def __init__(self,a) -> None:
        print("Initialise Dummy Process")
        
        
        
    def run(self) -> None:
        while True:
            time.sleep(1)
