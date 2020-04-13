# -*- coding: utf-8 -*-
import torch
import torch.nn.functional as F

import matplotlib.pyplot as plt

dtype = torch.float

    

def calPercentageCorrectness(net, x, y, title):
    net = net.cuda()
    net.eval()
    
    windowSize = y.shape[0]
    
    x = x.cuda()
    y = y.cuda()
    
    with torch.no_grad():
        
        
        
        hidden = (torch.zeros(1,windowSize,net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,windowSize,net.hiddenSize, dtype=dtype).cuda())
        
        net.hidden = hidden
        
        predicts = net(x).permute(1,0,2)
        
        
        values, indicies = predicts.max(axis=2)
        
        equalityCount = (indicies == y).sum().item()
        totalCount = y.shape[0] * y.shape[1]
        
        del hidden
        del predicts
        del x
        del y
        
        torch.cuda.empty_cache()
        
        
        
        print(title+" percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + " %")
        
    net.train()
    
    
def listValidatePerformanceWithThreshold(net, x, y, fileNames, threshold = 0.5):
    net = net.cuda()
    net.eval()
    
    windowSize = y.shape[0]
    
    with torch.no_grad():
        
        hidden = (torch.zeros(1,windowSize,net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,windowSize,net.hiddenSize, dtype=dtype).cuda())
        
        net.hidden = hidden
        
        predicts = net(x.cuda()).cpu().permute(1,0,2)
        
        values, indicies = predicts.max(axis=2)
        
        softmaxValues = F.softmax(predicts,dim=2).cpu()
        
        theValues = torch.zeros(indicies.shape[0], indicies.shape[1], dtype=torch.int64).cpu()
        
        for i,record in enumerate(indicies):
            for j,maxIndex in enumerate(record):
                if(softmaxValues[i][j][maxIndex] < threshold):
                    theValues[i][j] = 0
                else:
                    theValues[i][j] = maxIndex
                    
        validates = y.cpu()
        
        for i,record in enumerate(theValues):
            equalityCount = (record == validates[i]).sum().item()
            totalCount =  validates.shape[1]
            print("Validate percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + "% " + fileNames[i])
        
        del hidden
        del predicts
        del x 
        del y
        
        torch.cuda.empty_cache()
        
    net.train()    
    
    
def inspectValidatePerformanceByFileName(net, x, y, fileNames, targetFileName):
    
    net = net.cuda()
    
    targetIndex = -1
    for i,name in enumerate(fileNames):
        if(targetFileName in name):
            targetIndex = i
            break
    if(targetIndex is -1):
        print("File could not be found in validate files")
        return
    net.eval()
    
    windowSize = y.shape[0]

    
    with torch.no_grad():
        
        hidden = (torch.zeros(1,windowSize,net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,windowSize,net.hiddenSize, dtype=dtype).cuda())
        
        net.hidden = hidden
        
        predicts = net(x.cuda()).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        
        plt.figure()
        
        plt.plot(y[targetIndex].to("cpu"), "g", label="yTorchValidate")
        
        plt.plot(indicies[targetIndex].to("cpu"), "b", label="Predictions")
        
        plt.legend(loc="upper right")

        plt.show()
        
        del hidden
        del predicts
        del x 
        del y
        
        torch.cuda.empty_cache()
        
    net.train()
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    