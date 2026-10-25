import struct

class MessageStructure:
    def __init__(self, structureDict: dict[str, str]):
        self._config = structureDict
        self._indexStarts = {}
        self._indexEnds = {}
        self._objTypes = {}
        self.totalSize = 0
        self._myArray = bytearray()

        # Add Data definitions internally
        for key in structureDict:
            typev = structureDict[key]
            if typev[-1] == ']':
                typev, count = typev[:-1].split("[")
                count = int(count)
            else:
                count = 1

            self._addDatatype(key, typev, count)

    # Add a definition to the message and resize the buffer
    def _addDatatype(self, key: str, vtype: str, count: int):
        self._indexStarts[key] = self.totalSize
        self._objTypes[key] = vtype

        #print("Adding Type {} Length {} for {} index {}".format(vtype, count, key, self.totalSize))

        if (vtype == "int"):
            self.totalSize += 4 * count
        elif (vtype == "float"):
            self.totalSize += 4 * count
        elif (vtype == "char"):
            self.totalSize += 1 * count
        elif (vtype == "long"):
            self.totalSize += 8 * count
        elif (vtype == "string"):
            self.totalSize += count + 1
        elif (vtype == "bytes"):
            self.totalSize += count

        self._myArray = bytearray(self.totalSize)
        self._indexEnds[key] = self.totalSize


    def _getIndexOfDatablock(self, key: str) -> tuple[int,int]:
        return (self._indexStarts[key], self._indexEnds[key])
    
    #Although python supports multiple types, we break this out for parity sake
    #In fact, python makes this real nasty as we can't share pointers

    #Setters
    def setDouble(self, key: str, value: float, index: int = 0) -> None:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (8 * index)
        endIndex = bufferIndex+8
        self._myArray[bufferIndex:endIndex] = struct.pack('d', value)

    def setFloat(self, key: str, value: float, index: int = 0) -> None:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (4 * index)
        endIndex = bufferIndex+4
        self._myArray[bufferIndex:endIndex] = struct.pack('f', value)

    def setInt(self, key: str, value: int, index: int = 0) -> None:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (4 * index)
        endIndex = bufferIndex+4
        self._myArray[bufferIndex:endIndex] = struct.pack('i', value)

    def setLong(self, key: str, value: int, index: int = 0) -> None:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (8 * index)
        endIndex = bufferIndex+8
        self._myArray[bufferIndex:endIndex] = struct.pack('q', value)

    def setChar(self, key: str, value: bytes, index: int = 0) -> None:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (1 * index)
        endIndex = bufferIndex+1
        self._myArray[bufferIndex:endIndex] = value[0]

    def setBytes(self, key: str, value: bytes) -> None:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        self._myArray[bufferIndex:endIndex] = value

    def setString(self, key: str, value: str) -> None:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)

        for char in value:
            self._myArray[bufferIndex] = ord(char)
            bufferIndex += 1 

        self._myArray[bufferIndex] = 0 #Denote end of string with null terminator

    #Getters
    def getDouble(self, key: str, index: int = 0) -> float:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (8 * index)
        endIndex = bufferIndex+8
        return struct.unpack('d', self._myArray[bufferIndex:endIndex])[0]

    def getFloat(self, key: str, index: int = 0) -> float:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (4 * index)
        endIndex = bufferIndex+4
        return struct.unpack('f', self._myArray[bufferIndex:endIndex])[0]
    
    def getInt(self, key: str, index: int = 0) -> int:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (4 * index)
        endIndex = bufferIndex+4
        return struct.unpack('i', self._myArray[bufferIndex:endIndex])[0]

    def getLong(self, key: str, index: int = 0) -> int:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (8 * index)
        endIndex = bufferIndex+8
        return struct.unpack('q', self._myArray[bufferIndex:endIndex])[0]
    
    def getChar(self, key: str, index: int = 0) -> bytes:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        bufferIndex += (1 * index)
        endIndex = bufferIndex+1
        return self._myArray[bufferIndex:endIndex]
    
    def getBytes(self, key: str) -> bytes:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)
        return self._myArray[bufferIndex:endIndex]

    def getString(self, key: str) -> str:
        bufferIndex, endIndex = self._getIndexOfDatablock(key)

        string = ""
        index = bufferIndex
        for i in range(bufferIndex, endIndex):
            if self._myArray[index] == 0:
                break
            string += chr(self._myArray[index])
            index += 1
        return string
       

    #For communications, please don't touch unless you know what you are doing
    def getDecodeBuffer(self) -> bytes:
        return bytes(self._myArray)
    
    def setDecodeBuffer(self, buffer: bytes) -> None:
        self._myArray = bytearray(buffer)

    def clone(self):
        newStructure = MessageStructure(self._config)
        return newStructure


        

    