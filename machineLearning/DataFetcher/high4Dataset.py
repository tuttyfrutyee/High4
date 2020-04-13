import DataFetcher.decodeSData as Decoder
import os
import numpy as np
import random
import matplotlib.pyplot as plt


def getHigh4Dataset(recordFolderName):
    
    recordLength = 1650

    relativeFileNames = []
    fileNames = []

    for root,dirs,files in os.walk("./"+recordFolderName):
        for fileName in files:
            if("BIN" in fileName or "bin" in fileName):
                relativeFileNames.append(root + "/" + fileName)
                fileNames.append(fileName)
                
                
    records = []
    labels = []
    

    for fileName in relativeFileNames:
        
        print(fileName)
        moments, y = Decoder.decodeSensorData(fileName)

        records.append(moments[0:recordLength])
        labels.append(y[0:recordLength])

    
    records = np.stack(records, axis=0)
    labels = np.stack(labels, axis=0)
    
    return (records, labels, fileNames)

