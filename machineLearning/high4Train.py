import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import numpy as np

import os

import matplotlib.pyplot as plt
import matplotlib.patches as patches
plt.ioff()

import high4Dataset

import high4Net



def calTrainPercentageCorrectness():
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,batchSize,high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,batchSize,high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchTrain[:,0:batchSize,:]).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        equalityCount = (indicies == yTorchTrain[0:batchSize]).sum().item()
        totalCount = yTorchTrain.shape[1] * batchSize
        print("Train percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + " %")
    net.train()

def calValidatePercentageCorrectness():
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchValidate).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        equalityCount = (indicies == yTorchValidate).sum().item()
        totalCount = yTorchValidate.shape[0] * yTorchValidate.shape[1]
        print("Validate percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + " %")
    net.train()

def listValidatePerformances():
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchValidate).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        for i,indice in enumerate(indicies):
            equalityCount = (indice == yTorchValidate[i]).sum().item()
            totalCount =  yTorchValidate.shape[1]
            print("Validate percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + "% " + fileNamesValidate[i])
    net.train()

def inspectValidatePerformanceByFileName(fileName):
    targetIndex = -1
    for i,name in enumerate(fileNamesValidate):
        if(fileName in name):
            targetIndex = i
            break
    if(targetIndex is -1):
        print("File could not be found in validate files")
        return
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())
        net.hidden = hidden
        predicts = net(xTorchValidate).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        
        plt.plot(yTorchValidate[targetIndex].to("cpu"), "g", label="yTorchValidate")
        plt.plot(indicies[targetIndex].to("cpu"), "b", label="Predictions")
        plt.legend(loc="upper right")
        equality = (yTorchValidate[targetIndex] == indicies[targetIndex]).to("cpu")
#        for i,equal in enumerate(equality):
#            if(not equal):
#                plt.axvline(i,color="r")
        plt.show()
        
    net.train()

def listValidatePerformanceWithThreshold(threshold = 0.5):
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchValidate).cpu().permute(1,0,2)
        print(predicts.shape)
        values, indicies = predicts.max(axis=2)
        softmaxValues = F.softmax(predicts,dim=2).cpu()
        
        theValues = torch.zeros(indicies.shape[0], indicies.shape[1], dtype=torch.int64).cpu()
        
        for i,record in enumerate(indicies):
            for j,maxIndex in enumerate(record):
                if(softmaxValues[i][j][maxIndex] < threshold):
                    theValues[i][j] = 0
                else:
                    theValues[i][j] = maxIndex
                    
        validates = yTorchValidate.cpu()
        
        for i,record in enumerate(theValues):
            equalityCount = (record == validates[i]).sum().item()
            totalCount =  validates.shape[1]
            print("Validate percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + "% " + fileNamesValidate[i])
    net.train()
    
def inspectValidatePerformanceByFileNameWithThreshold(fileName, threshold=0.5):
    print(threshold)
    targetIndex = -1
    for i,name in enumerate(fileNamesValidate):
        if(fileName in name):
            targetIndex = i
            break
    if(targetIndex is -1):
        print("File could not be found in validate files")
        return
    net.eval()
    with torch.no_grad():
       
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchValidate).cpu().permute(1,0,2)
        values, maxIndicies = predicts.max(axis=2)
        softmaxValues = F.softmax(predicts,dim=2).cpu()
        theValues = torch.zeros(1, maxIndicies.shape[1], dtype=torch.int64).cpu()
        
        for j,maxIndex in enumerate(maxIndicies[targetIndex]):
            if(softmaxValues[targetIndex][j][maxIndex] < threshold):
                theValues[0][j] = 0
            else:
                theValues[0][j] = maxIndex
                
        plt.plot(yTorchValidate[targetIndex].to("cpu"), "g", label="yTorchValidate")
        plt.plot(theValues[0].to("cpu"), "b", label="Predictions")
        plt.legend(loc="upper right")

        plt.show()
        
    net.train()    
         
os.environ['CUDA_VISIBLE_DEVICES'] = '0'
dtype = torch.float
device = torch.device("cuda:0")

batchSize = 256
epochCount = 100000
trainRatio = 0.8



net = high4Net.High4Net().cuda()

classWeights = torch.from_numpy(np.array([1, 10,10, 10,6,6,10,6,6])).float().cuda()
criterion = nn.CrossEntropyLoss(weight=classWeights)
optimizer = optim.Adam(net.parameters(), lr = 1e-4)

seeds = [21,20,60,51,77,79,90,74]
seed2 = 11

xBag = []
labelBag = []


for i,seed in enumerate(seeds):
    x, labels, fileNames = high4Dataset.getHigh4Dataset(seed,seed2)
    
    if(i == 0):
        xBag = x[0:int(x.shape[0]*trainRatio)]
        xValidate = x[int(x.shape[0]*trainRatio):]
        labelBag = labels[0:int(labels.shape[0]*trainRatio)]
        labelsValidate = labels[int(labels.shape[0]*trainRatio):]
        fileNamesValidate = fileNames[int(fileNames.shape[0]*trainRatio):]
    else:
        xBag = np.concatenate((xBag,x[0:int(x.shape[0]*trainRatio)]),axis=0)
        labelBag = np.concatenate((labelBag,labels[0:int(labels.shape[0]*trainRatio)]),axis=0)

#shuffle time
seed = np.random.randint(0,100)
np.random.seed(seed)
np.random.shuffle(xBag) 
np.random.seed(seed)
np.random.shuffle(labelBag)
np.random.seed(seed)

xTrain = xBag
labelsTrain = labelBag

#reshaping
xTrainReshaped = np.swapaxes(xTrain,0,1)
xValidateReshaped = np.swapaxes(xValidate,0,1)

#make those torched
xTorchTrain = torch.from_numpy(xTrainReshaped).float().cuda()
xTorchValidate = torch.from_numpy(xValidateReshaped).float().cuda()

yTorchTrain = torch.from_numpy(labelsTrain-1).long().cuda()
yTorchValidate = torch.from_numpy(labelsValidate-1).long().cuda()


#create to hidden, since the last batch might not be divisible quite
lastHiddenSize = int(xBag.shape[0]*trainRatio) % batchSize
#for the divisible ones
hidden = (torch.zeros(1,batchSize,high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,batchSize,high4Net.hiddenSize, dtype=dtype).cuda())
#for the last one
if(lastHiddenSize != 0):
    hiddenLast = (torch.zeros(1,lastHiddenSize,high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,lastHiddenSize,high4Net.hiddenSize, dtype=dtype).cuda())

trainLosses = []

batchIterationCount = int(int(xBag.shape[0]*trainRatio) / batchSize)
if(lastHiddenSize != 0):
    batchIterationCount += 1

net.train()
for epoch in range(epochCount):
    
    for batchIndex in range(batchIterationCount):
        
        if(lastHiddenSize != 0 and batchIndex == (batchIterationCount-1)):
            net.hidden = hiddenLast
            slicingIndexLeft = int(xBag.shape[0]*trainRatio) - lastHiddenSize
            slicingIndexRight = int(xBag.shape[0]*trainRatio)
            
        else:
            net.hidden = hidden
            slicingIndexLeft = batchIndex * batchSize
            slicingIndexRight = (batchIndex+1) * batchSize
            
        predicts = net(xTorchTrain[:,slicingIndexLeft : slicingIndexRight]).permute(1,2,0)
    
        loss = criterion(predicts, yTorchTrain[slicingIndexLeft : slicingIndexRight,:])
        
        optimizer.zero_grad()
        
        if(epoch%100 == 0 and batchIndex == 0):
            print(epoch, loss.item())
            calTrainPercentageCorrectness()
            calValidatePercentageCorrectness()
    
        if(batchIndex == 0):
            trainLosses.append(loss.item())
            
        loss.backward()
        optimizer.step()    


plt.plot(trainLosses[-200:])
plt.show()
