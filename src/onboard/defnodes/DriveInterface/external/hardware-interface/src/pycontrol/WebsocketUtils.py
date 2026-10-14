#GNU General Public License v3.0
#Code by MegaKG
import hashlib
import base64
import TCPstreams5 as tcp
import struct
import socket
import time


#First Thing to say, WHAT AN EVIL PROTOCOL

#The Websocket Specification requires this UUID as a value
CONSTANT = b'258EAFA5-E914-47DA-95CA-C5AB0DC85B11'


#We get a key encoded in base64. We respond with the base64 concatenated with the Constant UUID and return the SHA1 digest
def genaccept(REQUEST):
    return base64.b64encode(hashlib.sha1(REQUEST.getHeader('Sec-WebSocket-Key').encode() + CONSTANT).digest()).decode()


#The Response header for Websocket
RESP = """HTTP/1.1 101 Switching Protocols
Upgrade: WebSocket
Connection: Upgrade
Sec-WebSocket-Accept: [ACCEPT]

"""

#Takes the decoded request options and generates a response header
def responseHeader(REQUEST):
    return RESP.replace('[ACCEPT]',genaccept(REQUEST))




#According to the specification:
"""
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-------+-+-------------+-------------------------------+
  |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
  |I|S|S|S|  (4)  |A|     (7)     |             (16/63)           |
  |N|V|V|V|       |S|             |   (if payload len==126/127)   |
  | |1|2|3|       |K|             |                               |
  +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
  |     Extended payload length continued, if payload len == 127  |
  + - - - - - - - - - - - - - - - +-------------------------------+
  |                               |Masking-key, if MASK set to 1  |
  +-------------------------------+-------------------------------+
  | Masking-key (continued)       |          Payload Data         |
  +-------------------------------- - - - - - - - - - - - - - - - +
  :                     Payload Data continued ...                :
  + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
  |                     Payload Data continued ...                |
  +---------------------------------------------------------------+

"""



#This wraps Websocket Connections with a similar interface to those of TCPstreams3 and TLSwrapper
class wrappedConnection(tcp.serverCon):
    def __init__(self,CON):
        self.conn = CON.conn
        self.info = CON.info

    #Parses the content of a websocket frame
    def _parsePacket(self,RawData):
        #Decode the first byte, containing the FIN and Opcode info
        B1 = RawData[0]

        #Get the Finish Flag Bit
        #128 = 10000000
        Fin = (B1 & 128) != 0 #Get the MSB (First Bit) and determine if it isn't zero

        #Get the Opcode
        #15 = 00001111
        OP = B1 & 15 #Get the Last 4 bits of the byte

        #Now get the Second significant Byte containing the Mask and Payload variable
        B2 = RawData[1]
        
        #Get the Mask Flag bit
        #128 = 10000000
        Mask = (B2 & 128) != 0 #Get the MSB (First Bit) and determine if it isn't zero

        #Get the Unsigned (char) 1 byte containing the length out of 127
        #127 = 01111111
        FirstLength = B2 & 127

        #This Length can have special meanings
        if FirstLength == 127:
            #64 Bit Length, need 8 Bytes
            #Format 'Q' is unsigned long integer (8 bytes)
            ActualLength = struct.unpack('!Q',RawData[2:10])[0]
            ContentStart = 10

        elif FirstLength == 126:
            #16 Bit Length, Need 2 Bytes
            ActualLength = struct.unpack('!H',RawData[2:4])[0]
            ContentStart = 4
        else:
            #7 Bit Length, do nothing as out of 127 anyway
            ActualLength = FirstLength
            ContentStart = 2

        #Get the masking key
        MaskingKey = RawData[ContentStart:ContentStart+4]
        #print("Masking Key",MaskingKey)

        #Return
        if not Mask:
            return {
                'IsEnd':Fin,
                'Content':RawData[ContentStart:],
                'Length':ActualLength,
                'OP':OP
            }
        else:
            #We now have to decode from where the actual content begins
            ContentStart += 4
            OutContent = bytearray()

            #We iterate over the content, using the XOR function with the relevant byte of the key
            KeyCounter = 0
            for char in RawData[ContentStart:]:
                OutContent.append( char ^ MaskingKey[KeyCounter] )
                KeyCounter += 1
                if KeyCounter == 4:
                    KeyCounter = 0

            return {
                'IsEnd':Fin,
                'Content':bytes(OutContent),
                'Length':ActualLength,
                'OP':OP
            }

    #Creates a websocket frame
    def _genpacket(self,DATA,opcode=1):
        #Get the Length of our data
        Length = len(DATA)

        #The output frame
        OutputFrame = bytearray()

        #attach the Finish Bit and the opcode
        OutputFrame.append( 128 | opcode )

        #Then attach the length 

        #If Smaller than 126
        if Length < 126:
            OutputFrame.append( Length )

        #If it is within 16 bit 
        elif Length < 65536:
            #Give the 126
            OutputFrame.append( 126 )
            #Now two bytes
            OutputFrame += struct.pack('!H',Length)
        
        #If it is 64 bit
        elif Length < 2**64:
            #Give the 127
            OutputFrame.append( 127 )
            #Now eight bytes
            OutputFrame += struct.pack('!Q',Length)

        else:
            raise ValueError("Invalid Length for Websocket Frame")

        return bytes(OutputFrame) + DATA

    def senddat(self,Data):
        try:
            #1 is an opcode for normal ascii text
            Processed = self._genpacket(Data,1)
            self.info['TotalSent'] += len(Processed)
            self.conn.send(Processed)
            return True
        except socket.error:
            self.close()
            return False

    def sendstdat(self,strdat):
      return self.senddat(strdat.encode('utf-8'))

    def getdat(self,buf=8192):
      GOT = b''
      while True:
        D = self.conn.recv(buf)
        if D == b'':
            self.close()
            return False
        
        Result = self._parsePacket(D)
        GOT += Result['Content']
        if Result['IsEnd']:
            break

      self.info['TotalRecv'] += len(GOT)
      self.info['LastPacket'] = time.time()
      return GOT

    def getstdat(self,buf=1024):
      GOT = self.getdat(buf)
      if GOT != False:
        return GOT.decode('utf-8')
      else:
        return GOT

    
