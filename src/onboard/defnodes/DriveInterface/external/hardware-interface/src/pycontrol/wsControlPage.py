import Pages
import json
import torque
import threading
import time

class page(Pages.webpage):
    def __init__(self, CanInterface):
        #Ensure that the page is initialised like its parent
        super().__init__(None)

        #Start the torque handler
        self.torqueHandler = torque.TorqueHandler(CanInterface)

        #Set the default drive mode
        self.torqueHandler.set_mode(torque.UNLOCKED_VELOCITY)

        #Calibrate the torque handler
        self.torqueHandler.calibrate()

        #Enable the torque handler
        self.torqueHandler.enable()


    def connect(self,Request: Pages.Connection):
        # Get the HTTP Content for testing
        f = open("pageData.html","r")
        MSG = f.read()
        f.close()

        #Send it to the client
        Request.sendCode(200)
        Request.sendLength(len(MSG))
        Request.sendType("text/html")

        Request.print(MSG)

    def websocket(self,Request: Pages.Connection):
        print("Socket Listener Running")
        CON = Request.getConnectionObject()

        #Time for some funky shit that Aston will kill me for - K

        def virtualStop():
            self.torqueHandler.set_speed(0,0)

        ButtonHandlers = {
            "enable": self.torqueHandler.enable,
            "disable": self.torqueHandler.disable,
            "calibrate": self.torqueHandler.calibrate,
            "stop": virtualStop,
            "estop": self.torqueHandler.estop
        }


        # A thread to periodically return wheel odom data
        def odomThreadFunction():
            while True:
                odom = self.torqueHandler.get_odom()
                print("Sync Odom",odom)
                time.sleep(0.5)

                CON.sendstdat(json.dumps({
                    "leftpos": odom[0],
                    "rightpos": odom[1],
                    "leftvel": odom[2],
                    "rightvel": odom[3]
                }))

        odomThread = threading.Thread(target=odomThreadFunction)
        odomThread.start()

        #The main connection loop
        while True:
            #Get the command in json format
            try:
                RawCMD = CON.getstdat()
                CMD = json.loads(RawCMD)
                print(CMD)
                commandName = CMD["command"]
            except Exception as E:
                print("Recv Error: ",E)
                continue

            #If it is a button press
            if commandName in ButtonHandlers:
                ButtonHandlers[commandName]()
            else:
                if commandName == "set_mode":
                    if CMD["mode"] == "Unlocked":
                        self.torqueHandler.set_mode(torque.UNLOCKED_VELOCITY)
                    elif CMD["mode"] == "Locked":
                        self.torqueHandler.set_mode(torque.LOCKED_VELOCITY)
                    elif CMD["mode"] == "Torque":
                        self.torqueHandler.set_mode(torque.UNLOCKKED_TORQUE)
            
                elif commandName == "set_speed":
                    self.torqueHandler.set_speed(float(CMD["left"]), float(CMD["right"]))
            