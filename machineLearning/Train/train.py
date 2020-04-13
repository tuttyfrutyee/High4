import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import numpy as np
import math

import matplotlib.pyplot as plt

import os

import Validation.StaticValidation.staticValidator as Validator


def train(modelGroup, data, epochCount, patience = 10, ):
    
    epoch = modelGroup["epoch"] 
    net = modelGroup["net"] 
    optimizer = modelGroup["optimizer"] 
    losses = modelGroup["losses"]
    classWeights = modelGroup["classWeights"]
    
    
    xTrainTorch = torch.from_numpy(np.swapaxes(data["xTrain"], 0,1)).float().cuda()
    yTrainTorch = torch.from_numpy(data["yTrain"]).long().cuda()
    
    xValTorch = torch.from_numpy(np.swapaxes(data["xVal"], 0,1)).float().cuda()
    yValTorch = torch.from_numpy(data["yVal"]).long().cuda()
        
    batchSize = 256
    
    if(data["xTrain"].shape[0] < batchSize):
        batchSize = data.data["xTrain"][0]
        
        
    os.environ['CUDA_VISIBLE_DEVICES'] = '0'
    dtype = torch.float
    device = torch.device("cuda:0")        
    
    
    #create to hidden, since the last batch might not be divisible quite
    lastHiddenSize = data["xTrain"].shape[0] % batchSize
    #for the divisible ones
    hidden = (torch.zeros(1,batchSize, net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,batchSize,net.hiddenSize, dtype=dtype).cuda())
    #for the last one
    if(lastHiddenSize != 0):
        hiddenLast = (torch.zeros(1,lastHiddenSize, net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,lastHiddenSize,net.hiddenSize, dtype=dtype).cuda())
        
        

    
    scheduler = optim.lr_scheduler.ReduceLROnPlateau(optimizer, 'min', patience = patience, factor=0.5, verbose=True)
    
    criterion = nn.CrossEntropyLoss(weight=classWeights)
    
    batchIterationCount = math.ceil(data["xTrain"].shape[0] / batchSize)
    
    
    net.train()
    
    firstTime = True
    
    
        
    
    for _ in range(epochCount):
        
        epoch += 1
        
        for batchIndex in range(batchIterationCount):
            
            if(lastHiddenSize != 0 and batchIndex == (batchIterationCount-1)):
                net.hidden = hiddenLast
                slicingIndexLeft = data["xTrain"].shape[0] - lastHiddenSize
                slicingIndexRight = data["xTrain"].shape[0]
                
            else:
                net.hidden = hidden
                slicingIndexLeft = batchIndex * batchSize
                slicingIndexRight = (batchIndex+1) * batchSize
                
            predicts = net(xTrainTorch[:,slicingIndexLeft : slicingIndexRight]).permute(1,2,0)
        
            loss = criterion(predicts, yTrainTorch[slicingIndexLeft : slicingIndexRight,:])
            
            del predicts
            torch.cuda.empty_cache()
            
            optimizer.zero_grad()
            
            if(epoch%100 == 0 and batchIndex == 0):
                
                print(epoch, loss.item())
                print(optimizer.param_groups[0]["lr"])
                
                modelGroup["epoch"] = epoch
                modelGroup["net"] = net
                modelGroup["optimizer"] = optimizer
                modelGroup["losses"] = losses
    
                Validator.calPercentageCorrectness(net, xTrainTorch[:,:batchSize], yTrainTorch[:batchSize], title="Train")
                Validator.calPercentageCorrectness(net, xValTorch, yValTorch, title="Validate")
                torch.cuda.empty_cache()
                
            if(batchIndex == 0):
                losses = np.append(losses,loss.item())
                if(firstTime and losses.shape[0]>300):
                    fig = plt.figure()
                    ax = fig.add_subplot(111)
                    line1, = ax.plot(losses[-300:], 'r-')
                    firstTime = False
                elif(losses.shape[0]>300):
                    line1.set_ydata(losses[-300:])
                    fig.canvas.draw()
                    fig.canvas.flush_events()
                    plt.ylim(np.min(losses[-400:]), np.max(losses[-400:]))   
                    
            loss.backward()
            optimizer.step()   
            
            if(batchIndex == 0):
                scheduler.step(loss.item())
    
    

    
