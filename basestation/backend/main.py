#!/usr/bin/env python3


__title__ = """
Equinox 2 Monothread Backend
Federated Rover Operation Gateway Runtime (F.R.O.G.R) V1.0

⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠔⢠⣄⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡠⠄⠂⠉⢀⣀⠀⠀⠉⡀
⠀⠀⠀⠀⠀⠀⢀⠀⠤⠀⠒⠀⠉⠀⠀⠀⠀⠀⡻⠋⢱⠀⠀⠇
⠀⠀⡀⢀⠔⠈⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠒⠁⢀⠞⠀
⢸⠁⠀⡎⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠄⠀⠀⢀⠴⠃⠀⠀
⠈⠑⠦⠀⠀⠀⠀⠀⠤⢀⠀⠀⠀⠀⡠⠂⠀⢀⠖⠁⠀⠀⠀⠀
⢀⠤⠒⠈⠉⠉⠀⠒⠂⠠⠌⠢⣀⣰⠀⠀⠀⡊⠀⢀⣀⡀⠀⠀
⠣⡀⠀⢀⡀⠀⠀⠀⠀⠀⠀⢀⣸⠝⣤⡀⠀⠀⠉⠀⠈⣧⠒⣢
⠀⠈⠒⠤⠬⣉⣀⠀⠀⠀⠉⢀⣙⠛⠾⡀⠉⠐⠒⠀⠐⠛⠂⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠉⠑⠢⢌⣀⠉⠐⠚⠀⠀⠀⠀⠀⠀⠀⠀


Authors:
- Kaelan Grainger (MegaKG)

"""

# System imports
import threading
import time

# Local imports
from AbstractProcess import AbstractProcess
from Config import Processes
import WSPageStub
from ModularWebServer import ModularWebServer



class main:
    def __init__(self) -> None:
        self.globalProcessDict = {}
        self.globalThreads = {}
        self.printLock = threading.Lock()

        # First start the local webserver
        self.webserver = ModularWebServer.webServer('0.0.0.0',8080,CatchAll=WSPageStub.wsPageStub())

        self.webserverThread = threading.Thread(target=self.webserver.run, name="FROGR WebSocket Server")
        self.webserverThread.start()

        for procName in Processes:
            print("Starting Thread ",procName)
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
    print('\033[0;32m',__title__,'\033[0m')
    mainApp().run()
