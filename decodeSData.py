
from struct import *

formatStringInt16 = "<1h" #to parse c int16_t (esp32 is little endian)
formatStringInt8 = "<1b" #to parse c int8_t (esp32 is little endian)

#First read the binary file and get bytes

""" with open("SDATA.BIN", "rb") as f:
    while True:
        chunk = f.read(4096)
        if not chunk:
            break
        for i in range(len(chunk)):
            if i == 0:
                allBytes = chunk
            else:
                allBytes = allBytes + chunk """

with open("SDATA.BIN", "rb") as f:

    byte_ = f.read(1)
    allBytes = [byte_]

    while byte_:
        # Do stuff with byte.
        byte_ = f.read(1)
        if byte_:
            allBytes.append(byte_)


#Detect the indexes of the recordings

#helper function

def getSingleByteValue(_byte):
    
    value = unpack(formatStringInt8,  _byte)
    
    return value[0]

def getMultipleInt16sFromBytes(_byteArray):
    
    willReturn = []
    
    for i in range(int((len(_byteArray)/2))):
        
        concatedBytes = _byteArray[i * 2] + _byteArray[i * 2 + 1]

        willReturn.append(unpack(formatStringInt16, concatedBytes))

    return willReturn

def getStream(_byteArray, imuCount):

    i = 0

    while(i < len(_byteArray)):
        if(i%2 == 0):
        
        else:

def checkSSequenceExist(index) : 
    if (index + 2) >= len(allBytes):
        return 0
    if(getSingleByteValue(allBytes[index]) == 0 and getSingleByteValue(allBytes[index+1]) == 0 and getSingleByteValue(allBytes[index+2]) == 0):
        return 1
    return 0


recordIndexes = []

index = 0


while index < len(allBytes):
    if(checkSSequenceExist(index)):
        recordIndexes.append(index)
        index = index + 3
        continue
    index = index + 1

# now seperate information from recordings
# each record has form : startSequence(0-0-0) , numberOfImu(1byte), currentModeIndicator(1byte), (12byte)*numberOfImu, timeElapsedBetweenAccData(1byte), (12byte)*numberOfImu ...

numberOfImus = []
currentModeIndicators = []
streams = [] #has the form: (12byte)*numberOfImu, timeElapsed, ...

for i in range(len(recordIndexes)):

    startIndex = recordIndexes[i]
    if i == (len(recordIndexes) -1):
        endIndex = len(recordIndexes)
    else:
        endIndex = recordIndexes[i+1]

    currentNumberOfImu = getSingleByteValue(allBytes[startIndex + 3])
    currentCurrentModeIndicator = getSingleByteValue(allBytes[startIndex + 4])
    currentLinAccRange = getSingleByteValue(allBytes[startIndex + 5])
    currentGyroRange = getSingleByteValue(allBytes[startIndex + 6])
    currentStream = getMultipleInt16sFromBytes(allBytes[startIndex+7 : startIndex + 11])

    print("stream")
    print(currentStream)