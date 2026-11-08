from abc import ABC, abstractmethod
import time

class AbstractProcess(ABC):
    # We expect through monkeypatching that the globalPrint function, global dict and process name will be set to replace these
    globalProcessDict = {}
    processName = "UNINITIALISED"
    printLock = None

    def globalPrint(self, *args, **kwargs) -> None:
            self.printLock.acquire()
            print("{} - [{}]: ".format(round(time.time(),2), self.processName), *args, **kwargs)
            self.printLock.release()

    print = globalPrint


    @abstractmethod
    def run(self) -> None:
        pass