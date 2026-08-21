import pyRover

IMU_ID = 0x5
imuMaster = pyRover.PyRover("can0",0)


class LEDCOLOUR:
    SAFE = 0
    MOTION = 1
    AUTO_PREP = 2
    AUTO = 3
    LOCKED = 4
    CONFLICT = 5
    ERROR = 6

def setLED(myColour: int):
    imuMaster.ToggleState(IMU_ID, myColour, 1)
