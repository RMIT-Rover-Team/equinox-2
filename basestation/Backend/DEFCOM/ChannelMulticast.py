import threading
from ._FlexibleMessageStructure import MessageStructure
from ._DefComParser import ConnectionSpecification as _ConnectionSpecification, loadConfFile as _loadConfFile
from ._TCPWrapper import clientTCPCon as _clientTCPCon, serverTCPCon as _serverTCPCon, newTCPServer as _newTCPServer
from ._UNIXWrapper import clientUnixCon as _clientUnixCon, serverUnixCon as _serverUnixCon, newUnixServer as _newUnixServer
import time
import os

class MulticastPublisher:
    #Initialise with the message structure definition (defcom file) and a function pointer
    #The function must be of the type:  void function(MessageStructure request, MessageStructure response)
    def __init__(self, defComFile: str):
        self._definition: _ConnectionSpecification = _loadConfFile(defComFile)

        #A mutex for the sending thread
        self._sendMutex = threading.Lock()

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
        
        #Outgoing buffer
        self._outgoingQueues = {}
        self.idCounter = 0

        self._acceptorThreadTCP.start()
        self._acceptorThreadUnix.start()


    #Get a message to be filled in
    def getNewMessageObject(self) -> MessageStructure:
        return self._definition.RequestMessageFormat.clone()

    def publish(self,requestData: MessageStructure):
        #Add the message to the outgoing queue
        with self._sendMutex:
            for key in self._outgoingQueues:
                self._outgoingQueues[key].append(requestData)

    # Internal Client creator
    def _spawnClient(self, client):
        #Start a new thread to handle it
        with self._sendMutex:
            clientNewThread = threading.Thread(target=self._clientProcess, args=(client,self.idCounter))
            clientNewThread.start()
            self._clientThreads.append(clientNewThread)
            self.idCounter += 1

            #Clean up dead clients
            ToCull = []
            for i in self._clientThreads:
                if not i.is_alive():
                    ToCull.append(i)
            for i in ToCull:
                self._clientThreads.remove(i)

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

            

            
            
    #The client handler thread
    def _clientProcess(self, con, myID):
        # Basically just wait for requests and provide responses
        Running = True
        with self._sendMutex:
            self._outgoingQueues[myID] = []

        while Running:
            if con.info["Alive"]:
                if len(self._outgoingQueues[myID]) > 0:
                    with self._sendMutex:
                        while len(self._outgoingQueues[myID]) > 0:
                            #Send the data
                            if not con.senddat(self._outgoingQueues[myID][0].getDecodeBuffer()):
                                Running = False

                            #Next message
                            self._outgoingQueues[myID] = self._outgoingQueues[myID][1:]
                else:
                    time.sleep(0.1)

        con.close()
        print("Client Disconnected {} {}".format(con.info["Address"]))

        #Delete the queue
        with self._sendMutex:
            del self._outgoingQueues[myID]




class MulticastSubscriber:
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
        

    def subscribe(self) -> MessageStructure:
        #Receive the response
        message = self._con.getdat(self._definition.RequestMessageFormat.totalSize)

        #Return the response
        response = self._definition.RequestMessageFormat.clone()
        
        if message:
            response.setDecodeBuffer(message)

        return response

    