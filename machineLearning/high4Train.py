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
         
os.environ['CUDA_VISIBLE_DEVICES'] = '0'
dtype = torch.float
device = torch.device("cuda:0")

batchSize = 32
epochCount = 10000
trainRatio = 0.8



net = high4Net.High4Net().cuda()

criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(net.parameters(), lr = 3e-4)

seeds = [2]

xBag = []
labelBag = []


for i,seed in enumerate(seeds):
    x, labels, fileNames = high4Dataset.getHigh4Dataset(seed)
    
    if(i == 0):
        xBag = x
        labelBag = labels
    else:
        xBag = np.concatenate((xBag,x),axis=0)
        labelBag = np.concatenate((labelBag,labels),axis=0)

#shuffle time
seed = np.random.randint(0,100)
np.random.seed(seed)
np.random.shuffle(xBag)
np.random.seed(seed)
np.random.shuffle(labelBag)
np.random.seed(seed)
np.random.shuffle(fileNames)

xTrain = xBag[0:int(xBag.shape[0]*trainRatio)]
xValidate = xBag[int(xBag.shape[0]*trainRatio):]

labelsTrain = labelBag[0:int(xBag.shape[0]*trainRatio)]
labelsValidate = labelBag[int(xBag.shape[0]*trainRatio):]

fileNamesTrain = fileNames[0:int(xBag.shape[0]*trainRatio)]
fileNamesValidate = fileNames[int(xBag.shape[0]*trainRatio):]
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


plt.plot(trainLosses[-100:])
plt.show()
