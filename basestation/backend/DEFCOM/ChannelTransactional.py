import threading
from ._FlexibleMessageStructure import MessageStructure
from ._DefComParser import ConnectionSpecification as _ConnectionSpecification, loadConfFile as _loadConfFile
from ._TCPWrapper import clientTCPCon as _clientTCPCon, serverTCPCon as _serverTCPCon, newTCPServer as _newTCPServer
from ._UNIXWrapper import clientUnixCon as _clientUnixCon, serverUnixCon as _serverUnixCon, newUnixServer as _newUnixServer
import time
import os

class ServerChannel:
    #Initialise with the message structure definition (defcom file) and a function pointer
    #The function must be of the type:  void function(MessageStructure request, MessageStructure response)
    def __init__(self, defComFile: str, handlerHook: callable):
        self._definition: _ConnectionSpecification = _loadConfFile(defComFile)

        #As I have no faith in people writing thread safe code, we mutex the hook
        self._hookMutex = threading.Lock()
        self._hook: callable = handlerHook

        #Ensures that only one spawn at a time
        self._acceptMutex = threading.Lock()
  

        #Open the tcp server on the decoded port and address
        self._tcpServer = _newTCPServer(self._definition.ResolvedIP, self._definition.NumericPort)

        #Open a named socket as well
        self._unixServer = _newUnixServer('/tmp/' + self._definition.Name)

        #The acceptor thread for TCP
        self._acceptorThreadTCP = threading.Thread(target=self._acceptorTCP)

        #The acceptor thread for Unix
        self._acceptorThreadUnix = threading.Thread(target=self._acceptorUnix)

        #The client thread array
        self._clientThreads = []

    # Internal thread that accepts client connections and spins off new threads for them
    def _acceptorTCP(self):
        while True:
            #Accept a client
            client = _serverTCPCon(self._tcpServer)
            print("Accepted TCP Client {}".format(client.info["Address"]))

            self._spawnClient(client)

    def _acceptorUnix(self):
        while True:
            #Accept a client
            client = _serverUnixCon(self._unixServer)
            print("Accepted UNIX Client {}".format(client.info["Address"]))

            self._spawnClient(client)

    # Internal Client creator
    def _spawnClient(self, client):
        #Start a new thread to handle it
        with self._acceptMutex:
            clientNewThread = threading.Thread(target=self._clientProcess, args=(client,))
            clientNewThread.start()
            self._clientThreads.append(clientNewThread)

            #Clean up dead clients
            ToCull = []
            for i in self._clientThreads:
                if not i.is_alive():
                    ToCull.append(i)
            for i in ToCull:
                self._clientThreads.remove(i)

    #The client handler thread
    def _clientProcess(self, con):
        # Basically just wait for requests and provide responses
        while True:
            if con.info["Alive"]:
                message = con.getdat(self._definition.RequestMessageFormat.totalSize)

                if not message: #Null message is broken connection
                    break

                #Copy the requets and reply objects so we can mess with them
                LocalRequest = self._definition.RequestMessageFormat.clone()
                LocalResponse = self._definition.ResponseMessageFormat.clone()

                #Copy the message into the buffer
                LocalRequest.setDecodeBuffer(message)

                #Zero the response buffer
                LocalResponse.setDecodeBuffer(bytes(self._definition.ResponseMessageFormat.totalSize))

                #Lock the hook mutex
                with self._hookMutex:
                    #Call the hook
                    self._hook(LocalRequest,LocalResponse)

                #Reply with the response
                if not con.senddat(LocalResponse.getDecodeBuffer()):
                    break #If the send fails we die
            else:
                break
        con.close()
        print("Client Disconnected {}".format(con.info["Address"]))

    #Starts everything running explicitly - this may be redundant tbh
    def start(self):
        self._acceptorThreadTCP.start()
        self._acceptorThreadUnix.start()



class ClientChannel:
    def __init__(self, defComFile: str):
        self._definition: _ConnectionSpecification = _loadConfFile(defComFile)

        #Connect to the server
        #If we are running on the same machine
        if self._definition.Name in os.listdir('/tmp'):
            self._con = _clientUnixCon('/tmp/' + self._definition.Name)
            print("Connected Unix to {}".format(self._definition.Name))
        else:
            self._con = _clientTCPCon(self._definition.ResolvedIP, self._definition.NumericPort)
            print("Connected TCP to {} port {}".format(self._definition.ResolvedIP, self._definition.NumericPort))

    #Get a request to be filled in
    def getNewRequestObject(self) -> MessageStructure:
        return self._definition.RequestMessageFormat.clone()

    def request(self,requestData: MessageStructure) -> MessageStructure:
        #Send the data
        self._con.senddat(requestData.getDecodeBuffer())

        #Receive the response
        message = self._con.getdat(self._definition.ResponseMessageFormat.totalSize)

        #Return the response
        response = self._definition.ResponseMessageFormat.clone()
        
        if message:
            response.setDecodeBuffer(message)

        return response