# rmit-lib-universal-canbus
A universal canbus wrapper for RAT series rovers

Contains several versions for varuous uses:

C++:
Linux - Master (SocketCan)
Linux - Slave (SocketCan)

Embedded - Slave (RATCAN)

Python:
Linux - Master (SocketCan)


Run buildPycontrol.sh to build control binary
Then copy dist/pyRover* to your python workdir

Python Reference:
```
import pyRover
master = pyRover.PyRover("can0",1) #Interface, Master ID
```

Ref:
master.BroadcastDataPoint() -> Tuple (Dict, Float) Gets the latest datapoint that has been broadcast
master.RequestDataPoint(ID, StreamID, ChannelID) -> Float (for science / electrical)
master.Calibrate(ID, MotorID) -> Boolean
master.SetMotorPosition(ID, MotorID, Value) -> Dict [Ignore this though]
master.EStop(ID) -> Boolean
master.SetMotorSpeed(ID, MotorID, Value) -> Dict [Ignore this though]
master.GetMotorPosition(ID, MotorID) -> Tuple (Dict, Float) where dict is Status
master.ToggleState(ID, ChannelID, Value) -> Dict
master.GetMotorSpeed(ID, MotorID) -> Tuple (Dict, Float) where dict is Status      
master.ping(ID) -> Boolean (Health check)
