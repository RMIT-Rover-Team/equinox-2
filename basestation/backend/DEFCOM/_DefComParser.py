from dataclasses import dataclass
from ._FlexibleMessageStructure import MessageStructure
import socket
import io



    
    

@dataclass
class ConnectionSpecification:
    #Connection Orientation
    ResolvedIP: str
    NumericPort: int
    Name: str
    Priority: int

    #The Request / Multicast message (main payload for Multicast)
    #Format - dictionary of [key: value] like ["VariableName":"VariableType"]
    RequestMessageFormat: MessageStructure

    #The Response message for Transactional connections
    ResponseMessageFormat: MessageStructure


#Hash string names into ports (to make naming easier)
def nameToPort(name: str) -> int:
    count = 0
    for c in name:
        count = (count * 31 + ord(c)) % ((1 << 16) - 1);
    return 2000 + (count % 60000)

#Resolve a FQDN to an IP address
def resolveFQDN(fqdn: str) -> str:
    return socket.gethostbyname(fqdn)

#Load up a file
def _recursiveLoad(f: io.TextIOWrapper) -> dict[str, str]:
    LoadedData = {}

    while True:
        line = f.readline()
        
        #End of file
        if line == "":
            break

        #Strip the whitespace
        line = line.strip().replace(" ","").replace("\t","")

        #If the line is now empty, ignore it
        if line == "":
            continue

        #End of segment
        if line[-1] == "}":
            break
        
        #Ignore comments
        if line[0] == "#":
            continue

        #Start of recursive Segment
        if line[-1] == "{":
            LoadedData[line.replace("{","")] = _recursiveLoad(f)
            continue

        #Otherwise we split into key and value
        key, value = line.split(":")
        LoadedData[key] = value
    return LoadedData

def loadConfFile(filename: str) -> ConnectionSpecification:
    f = open(filename, "r")

    #The overall dictionary
    LoadedData = _recursiveLoad(f)

    f.close()

    #Decode the IP and port
    RawPort, RawFQDN = LoadedData["Name"].split("@")
    IP = resolveFQDN(RawFQDN)

    if RawPort.isdigit():
        PORT = int(RawPort)
    else:
        PORT = nameToPort(RawPort)

    #Load the Request Message Format
    RequestMessageFormat = MessageStructure(LoadedData["RequestFormat"])
    
    #Do the same for the Response
    ResponseMessageFormat = MessageStructure(LoadedData["ResponseFormat"])

    # Add the priority
    if "Priority" in LoadedData:
        Priority = int(LoadedData["Priority"])
    else:
        Priority = 0

    print("Connection {} Specified on {} port {} Req Size {} Resp Size {}".format(LoadedData["Name"],IP, PORT, RequestMessageFormat.totalSize, ResponseMessageFormat.totalSize))
    return ConnectionSpecification(IP, PORT, RawPort, Priority, RequestMessageFormat, ResponseMessageFormat)

    
