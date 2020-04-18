# -*- coding: utf-8 -*-
import random
import time
import numpy as np

import DataFetcher.high4Dataset as Dataset


records = None
labels = None
fileNames = None




def fetchAllData(recordFolderName):
    
    global records
    global labels
    global fileNames
    
    records, labels, fileNames = Dataset.getHigh4Dataset(recordFolderName)
    
    return (records, labels, fileNames)
    
    
    
def getDataForFingers(fingers, trainRatio, multiplicity, seperationSeed = None):
    
    global records
    global labels
    global fileNames
    
    """
        Documentation:
            fingers : [fingerNumber],
            fingerNumber: possiblevalues : 0,1,2,3 -> 0:thumb, 1:fore finger, 2:middle finger, 3:ring finger
    """
    
    if(seperationSeed is None):
        seperationSeed = int(time.time())        
    
    
    records_ = None
    labels_ = np.copy(labels)
    fileNames_ = np.copy(fileNames)
      

    for finger in fingers:
        leftSlice = finger * 6
        rightSlice = leftSlice + 6
        if(records_ is None):
            records_ = records[:,:,leftSlice:rightSlice]
        else:
            records_ = np.concatenate((records_, records[:,:,leftSlice:rightSlice]), axis=-1)

    xVal = [] 
    yVal = []
    
    xTrainSample = [] #will create multiples from this sample
    yTrainSample = []

    #shuffle the records here with the seperationSeed
    np.random.seed(seperationSeed)
    np.random.shuffle(records_)
    np.random.seed(seperationSeed)
    np.random.shuffle(labels_)
    np.random.seed(seperationSeed)
    np.random.shuffle(fileNames_)
    
    
    
    seperationIndex = int((1-trainRatio) * records_.shape[0])
    
    xVal = records_[0:seperationIndex]
    yVal = labels_[0:seperationIndex]
    valFileNames = fileNames_[0:seperationIndex]
    
    xTrainSample = records_[seperationIndex:]
    yTrainSample = labels_[seperationIndex:]


    xTrain = None
    yTrain = None
    
    windowLength = 1400
    
    for i in range(multiplicity):
        leftStartIndex = np.random.randint(0, xTrainSample.shape[1] - windowLength)
        duplicateXTrain = np.copy(xTrainSample)[:,leftStartIndex: leftStartIndex + windowLength,:]
        duplicateYTrain = np.copy(yTrainSample)[:,leftStartIndex: leftStartIndex + windowLength]
        if(xTrain is None):
            xTrain = duplicateXTrain
            yTrain = duplicateYTrain
        else:
            xTrain = np.concatenate((xTrain, duplicateXTrain))
            yTrain = np.concatenate((yTrain, duplicateYTrain))
    
    np.random.seed(seperationSeed)
    np.random.shuffle(xTrain)
    np.random.seed(seperationSeed)
    np.random.shuffle(yTrain)
    
#    trainStatistics = {"accMean" : 0, "accStd" : 0, "gyroMean" : 0, "gyroStd": 0}
#    valStatistics = {"accMean" : 0, "accStd" : 0, "gyroMean" : 0, "gyroStd": 0}
#    
#    xTrain = normalizerBetter(xTrain, trainStatistics)
#    xVal = normalizerBetter(xVal, valStatistics)
##    xTrain = normalizerWorse(xTrain)
##    xVal = normalizerWorse(xVal)
#    
#    print(trainStatistics)
    
    return {"xTrain": xTrain, "yTrain":yTrain, "xVal":xVal, "yVal":yVal, "valFileNames":valFileNames}

def normalizerWorse(x):
    x = (x - x.mean()) / x.std()
    return x

def normalizerBetter(x, statistics):
    
    xAcc = []
    
    xGyro = []
    
    for i,moment in enumerate(x):
        for k in range(moment.shape[1]):
            
            l = k % 6
            
            if(l < 3):
                xAcc.append(moment[:,k])
            else:
                xGyro.append(moment[:,k])
                

    accMean = np.mean(xAcc)
    accStd = np.std(xAcc)
    gyroMean = np.mean(xGyro)
    gyroStd = np.std(xGyro)                
    
    statistics["accMean"] = accMean
    statistics["accStd"] = accStd
    statistics["gyroMean"] = gyroMean
    statistics["gyroStd"] = gyroStd
    
    for i,moment in enumerate(x):
        for k in range(moment.shape[1]):
            
            l = k % 6
                
            if(l < 3):
                x[i][:][k] = (x[i][:][k] - accMean) / accStd
            else:
                x[i][:][k] = (x[i][:][k] - gyroMean) / gyroStd             
    

    return x





