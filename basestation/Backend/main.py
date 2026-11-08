#!/usr/bin/env python3


"""
Equinox 2 Monothread application V1.0

Authors:
- Kaelan Grainger (MegaKG)

"""

# System imports
import threading
import time

# Local imports
from AbstractProcess import AbstractProcess
from Config import Processes



class mainApp:
    def __init__(self) -> None:
        self.globalProcessDict = {}
        self.globalThreads = {}
        self.printLock = threading.Lock()


        for procName in Processes:
            procObjUnInitialised = Processes[procName][0]

            # MonkeyPatch
            procObjUnInitialised.globalProcessDict = self.globalProcessDict
            procObjUnInitialised.processName = procName
            procObjUnInitialised.printLock = self.printLock

            # Now we initialise the class
            newProcObj = procObjUnInitialised(**Processes[procName][1])
            self.globalProcessDict[procName] = newProcObj

            # Start the process
            self.globalThreads[procName] = threading.Thread(target = newProcObj.run, name=procName)
            self.globalThreads[procName].start()

    def run(self) -> None:
        while True:
            time.sleep(1)

            # Check if all processes are done
            unfinishedFlag = False
            for procName in self.globalThreads:
                if self.globalThreads[procName].is_alive():
                    unfinishedFlag = True
                    break

            if not unfinishedFlag:
                break



if __name__ == "__main__":
    mainApp().run()
