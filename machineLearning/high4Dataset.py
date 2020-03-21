from decodeSData import *
import os
import numpy as np
import random
import matplotlib.pyplot as plt

recordIndex = 17


def getHigh4Dataset(seed):

    recordLength = 1200

    random.seed(seed)

    relativeFileNames = []
    fileNames = []

    for root,dirs,files in os.walk("./records"):
        for fileName in files:
            if("BIN" in fileName or "bin" in fileName):
                relativeFileNames.append(root + "/" + fileName)
                fileNames.append(fileName)
                
    records = []
    z = []

    print(len(relativeFileNames))

    for fileName in relativeFileNames:
        moments, y, modeNumber = decodeSensorData(fileName)

        records.append(np.array((moments, y)))
        z.append(modeNumber)

    for i,record in enumerate(records):
        totalLength = record.shape[1]

        randomLeft = random.randrange(0,totalLength-recordLength-2,1)
        randomRight = randomLeft + recordLength

        records[i] = records[i][:, randomLeft:randomRight]

    x = []
    y = []
    for record in records:
        tempX = []
        for moment in record[0]:
            tempX.append(moment)
        x.append(tempX)
        y.append(record[1])


    x = np.array(x)
    y = np.array(y)
    z = np.array(z)
    z = z.reshape((z.shape[0],1))

    labels = y*z

    for j,label in enumerate(labels):
        for i,value in enumerate(label):
            if(value == 0):
                labels[j,i] = 1

    print(x.shape, labels.shape)


    return (x, labels.astype(np.float64), fileNames)

    accMagnitudeFirstImu = []
    gyroMagnitudeFirstImu = []

    

    moments = x[recordIndex]


    print(y[recordIndex].sum())

    leftIndex = -1
    rightIndex = -1

    for i,value in enumerate(y[recordIndex]):
        if(value == 1 and leftIndex == -1):
            leftIndex = i 
        if(value == 0 and leftIndex != -1 and rightIndex == -1):
            rightIndex = i
       
    for moment in moments:
        accMagnitudeFirstImu.append(np.sqrt(moment[0]**2 + moment[1]**2 + moment[2]**2))
    
    plt.plot(accMagnitudeFirstImu)
    plt.axvline(x=leftIndex, color="red")
    plt.axvline(x=rightIndex, color="red")
    plt.show(block=True)


