#GNU General Public License v3.0
#Code by MegaKG
import mimetypes
import RequestParsers

#This file contains the base class for all webpages to be based off

class Connection:
    def __init__(self,CON,ParsedData):
        self._Connection = CON
        self._ParsedData = ParsedData

    def getConnectionObject(self):
        return self._Connection
    
    def getRequestHeader(self) -> RequestParsers.decodedRequest:
        return self._ParsedData
    
    #Me First
    def sendCode(self, Code):
        self._Connection.sendstdat("HTTP/1.1 " + str(Code) + " \nServer: KPython V4\nVary: Accept-Encoding\n")

    #Me Last
    def sendType(self,Mime):
        self._Connection.sendstdat("Content-Type: " + Mime + "\n\n")

    #Required for continual connections
    def sendLength(self,Length):
        self._Connection.sendstdat("Content-Length: " + str(Length) + "\n")

    #Custom Headers
    def sendHeader(self,Key,Value):
        self._Connection.sendstdat("{}: {}\n".format(Key,Value))

    #Create a cookie
    def setCookie(self,Key,Value,additionalOptions = []):
        if len(additionalOptions) > 0:
            Data = "Set-Cookie", "{}={};".format(Key,Value)
            for option in additionalOptions:
                Data += option + ';'
            self.sendHeader(Data)
        else:
            self.sendHeader("Set-Cookie", "{}={}".format(Key,Value))

    #Makes life easy for page generation
    def print(self,*IN):
        OUTST = ''
        for i in IN:
            OUTST += str(i) + ' '
        OUTST = OUTST[:-1].encode('utf-8')
        self._Connection.senddat(OUTST)

    def close(self):
        self._Connection.close()


class webpage:
    #This shouldn't ever be overwritten by the child
    def __init__(self,Options):
        self.Options = Options

    
    def mimeFromFileExtension(self,Path):
        return mimetypes.guess_type(Path)

    #These are User replacable functions in the child classes
    def connect(self,Request: Connection):
        pass

    def webSocket(self,Request: Connection):
        self.close()

    