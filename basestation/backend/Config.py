"""
Config file for Equinox 2 Monothread application
"""

from Processes import DummyProcess
from Processes import DriveProcess
from Processes import ExcavatorProcess

# Enter the objects that will be launched
# Format is: "ProcessName": [ ProcessObject, ArgsDict ]
# ArgsDict is a dictionary of arguments to be passed to the process 
Processes = {
    "Dummy": (DummyProcess.DummyProcess, {"a": 10}),
    "Drive": (DriveProcess.DriveProcess, {"maxDriveSpeed": 100}),
    "Excavator": (ExcavatorProcess.ExcavatorProcess, {})
}
