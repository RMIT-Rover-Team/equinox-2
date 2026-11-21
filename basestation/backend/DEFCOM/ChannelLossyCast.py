import threading
from ._FlexibleMessageStructure import MessageStructure
from ._DefComParser import ConnectionSpecification as _ConnectionSpecification, loadConfFile as _loadConfFile
from ._MultiUDPWrapper import udpsend, udpget

import time
import os
import struct

class LossyCastPublisher:
    #Initialise with the message structure definition (defcom file) and a function pointer
    #The function must be of the type:  void function(MessageStructure request, MessageStructure response)
    def __init__(self, defComFile: str):
        self._definition: _ConnectionSpecification = _loadConfFile(defComFile)

        if self._definition.RequestMessageFormat.totalSize > 1000:
            raise Exception('Message size too large for UDP')
        
        self._con = udpsend(self._definition.ResolvedIP, self._definition.NumericPort, self._definition.RequestMessageFormat.totalSize + 8)

        self.sequence = 63

        # Assign the priority
        self._con.setPriority(self._definition.Priority)

    #Get a message to be filled in
    def getNewMessageObject(self) -> MessageStructure:
        return self._definition.RequestMessageFormat.clone()

    def publish(self,requestData: MessageStructure):
        #Get the buffer
        buffer = requestData.getDecodeBuffer()
        buffer = struct.pack('Q',self.sequence) + buffer

        self._con.senddat(buffer)

        #Add a sequence header to it
        self.sequence += 1
        


    


class LossyCastSubscriber:
    def __init__(self, defComFile: str):
        self._definition: _ConnectionSpecification = _loadConfFile(defComFile)

        if self._definition.RequestMessageFormat.totalSize > 1000:
            raise Exception('Message size too large for UDP')

        #Connect to the server
        self._con = udpget(self._definition.ResolvedIP, self._definition.NumericPort, self._definition.RequestMessageFormat.totalSize + 8)

        self.lastSequence = 0
        

    def subscribe(self) -> MessageStructure:
        #Receive the response
        while True:
            message = self._con.getdat()

            #Get the sequence
            sequence = struct.unpack('Q',message[:8])[0]

            # Sequencing is scary, thankfully some engineers smarter than me worked this out
            # Out sequence ring is 64 with 32 being the division between old and new
            # This means that it can take max 32 packets for the connection to recover after loss
            if ((sequence - self.lastSequence) % 64) < 32: 
                self.lastSequence = sequence
                message = message[8:]
                break
            
        #Return the response
        response = self._definition.RequestMessageFormat.clone()
        
        if message:
            response.setDecodeBuffer(message)

        return response

    