
from struct import *
import sys
import numpy as np
import matplotlib.pyplot as plt

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




formatStringInt16 = "<1h" #to parse c int16_t (esp32 is little endian)
formatStringInt8 = "<1b" #to parse c int8_t (esp32 is little endian)


g = 9.807

def decodeSensorData(fileName): # returns x and y, x : [moment], y : [y(moment)], moment : vector of length numberOfImus * 6
# moment for one imu is consists of accX,accY,accZ, radX, radY, radZ where unit(acc) = m/s^2, unit(rad) = °/s

    with open(fileName, "rb") as f:

        byte_ = f.read(1)
        allBytes = [byte_]

        while byte_:
            # Do stuff with byte.
            byte_ = f.read(1)
            if byte_:
                allBytes.append(byte_)


    #Detect the indexes of the recordings


    """ 
    Documentation:

    Each file only contains data stream for one record. There is one record file for each record

    * File structure : numberOfImus(1byte) - currentMode(1byte) - linRangeBox[linRange](1byte) - radRangeBox[radRange](1byte) -

    * repeatTillFireSequence { dataStream(numberOfImus * 6 * 2 bytes) - timeDiff(1byte)} -

    * fireSequence(3bytes : 0,1,2) - 

    * repeatTillEndOfRecord { dataStream(numberOfImus * 6 * 2 bytes) - timeDiff(1byte)} -

    * dataStream(numberOfImus * 6 * 2 bytes) // one last time
    
    """

    configInfo = []

    dataStream = []

    """ 
    Documentation:

    -------------------
    'dataStream' is consists of data // array of arrays

    Data structure : A vector that is, for i in range(numberOfImus) data += [axi, ayi, azi, rxi, ryi, rzi]

        where ax : linear acc in x direction, rx rotational acc around x axis
    -------------------

    -------------------

    'configInfo' contains:

        numberOfImus, currentMode, linRangeBox, radRangeBox

    -------------------

    """

    byteIndex = 0
    indexEndConfig = 0
    #fill configInfo first

    # get numberOfImu
    configInfo.append(getSingleByteValue(allBytes[byteIndex]))
    byteIndex+=1
    # get currentMode
    configInfo.append(getSingleByteValue(allBytes[byteIndex]))
    byteIndex+=1
    #get linRangeBox
    configInfo.append(getSingleByteValue(allBytes[byteIndex]))
    byteIndex+=1
    #get radRangeBox
    configInfo.append(getSingleByteValue(allBytes[byteIndex]))
    byteIndex+=1 

    indexEndConfig = byteIndex


    #fill rawData

    #first detect the index of firesquence

    fireSequenceIndex = byteIndex
    catchCount = 0 # this should be len(fireSquence) in order to confirm firesquence
    fireSequence = [1,2,0,3,4,0]
    previousEncounterIndex = -1
    while(fireSequenceIndex < len(allBytes)):

        if(getSingleByteValue(allBytes[fireSequenceIndex]) == fireSequence[catchCount]):

            if(((previousEncounterIndex + 1) == fireSequenceIndex) or catchCount == 0):

                previousEncounterIndex = fireSequenceIndex
                catchCount+= 1
                if(catchCount == len(fireSequence)):
                    break
            else:
                catchCount = 0
        else:
            if(getSingleByteValue(allBytes[fireSequenceIndex]) == fireSequence[0]):
                catchCount = 1
                previousEncounterIndex = fireSequenceIndex
            else:
                catchCount = 0

        fireSequenceIndex+=1

    #now get data until firesquence // note that firesquence is 3 bytes long, fireSquence contains allBytes[fireSquenceIndex - 2, fireSquenceIndex - 1, fireSquenceIndex]
    beforeFireIndexLeft = indexEndConfig
    beforeFireIndexRight = fireSequenceIndex - len(fireSequence)
    afterFireIndexLeft = fireSequenceIndex + 1
    afterFireIndexRight = len(allBytes) - 1

    beforeFireMomentCount = (beforeFireIndexRight - beforeFireIndexLeft + 2) / (configInfo[0] * 12 + 1)
    afterFireMomentCount = (afterFireIndexRight - afterFireIndexLeft + 1) / (configInfo[0] * 12 + 1)

    moments = []
    sizeOfAnMoment = configInfo[0] * 12

    #first put the moments before the firesquence
    for i in range(int(beforeFireMomentCount)):
        moment = []
        for j in range(configInfo[0]):

            if(i == 0): # moment = 0 -> there is no time(1byte) before it
                targetIndexLeft = beforeFireIndexLeft + j * 12
                dataArray = getMultipleInt16sFromBytes(allBytes[targetIndexLeft: targetIndexLeft+12])
                for i,data in enumerate(dataArray):
                    if(i<3):
                        dataArray[i] = data[0] / (65536.0 / 2.) * configInfo[2] * g
                    else:
                        dataArray[i] = data[0] / (65536.0 / 2.) * configInfo[3] * 250 
            else:
                targetIndexLeft = beforeFireIndexLeft + (i-1) * (sizeOfAnMoment + 1) + 1 + sizeOfAnMoment + j*12
                dataArray = getMultipleInt16sFromBytes(allBytes[targetIndexLeft: targetIndexLeft+12])
                for i,data in enumerate(dataArray):
                    if(i<3):
                        dataArray[i] = data[0] / (65536.0 / 2.) * configInfo[2] * g
                    else:
                        dataArray[i] = data[0] / (65536.0 / 2.) * configInfo[3] * 250                 

            moment.append(dataArray)
        
        moment = np.array(moment).reshape(configInfo[0] * 6)    
        moments.append(moment)

    #secondly put the moments after the firesquence
    for i in range(int(afterFireMomentCount)):
        moment = []
        for j in range(configInfo[0]):


            targetIndexLeft = afterFireIndexLeft + i * (sizeOfAnMoment + 1) + 1 + j*12
            dataArray = getMultipleInt16sFromBytes(allBytes[targetIndexLeft: targetIndexLeft+12])
            for i,data in enumerate(dataArray):
                if(i<3):
                    dataArray[i] = data[0] / (65536.0 / 2.) * configInfo[2] * g
                else:
                    dataArray[i] = data[0] / (65536.0 / 2.) * configInfo[3] * 250             

            moment.append(dataArray)

        moment = np.array(moment).reshape(configInfo[0] * 6)   
        moments.append(moment)

    y = [0] * (int)(beforeFireMomentCount + afterFireMomentCount)

    if(configInfo[1] != 1):
        for i in range(int(beforeFireMomentCount) + 150 - 25, int(beforeFireMomentCount) + 150 + 75):
            y[i] = 1



    return (moments, y, configInfo[1])

    """ accMagnitudeFirstImu = []
    gyroMagnitudeFirstImu = []

    for moment in moments:
        magnitudeAcc = np.sqrt(moment[1][0]**2 + moment[1][1]**2 + moment[1][2]**2)
        magnitudeGyro = np.sqrt(moment[0][3]**2 + moment[0][4]**2 + moment[0][5]**2)
        accMagnitudeFirstImu.append(magnitudeAcc)
        gyroMagnitudeFirstImu.append(magnitudeGyro)
    #print(accMagnitudeFirstImu)
    plt.plot(accMagnitudeFirstImu)
    plt.axvline(x=beforeFireMomentCount, color="red")
    plt.show(block=True)
    """


