import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import numpy as np
import math

import os

import matplotlib.pyplot as plt
import matplotlib.patches as patches
plt.ioff()

import high4Dataset

import high4Net




def train(model, data, patience, epochCount ):
    
    batchSize = 256
    
    xTrain = data.xTrain
    yTrain = data.yTrain
    
    
    batchIterationCount = math.ceil(xTrain.shape[0] / batchSize)
    
    
    model.net.train()
    
    for epoch in range(epochCount):
        
        for batchIndex in range(batchIterationCount):
            
            slicingIndexLeft = batchSize * batchIndex
            slicingIndexRight = batchSize * (batchIndex+1)
            
            if(slicingIndexRight > xTrain.shape[0]):
                    slicingIndexRight = xTrain.shape[0] - 1
            
            predicts = model.net(xTrain[slicingIndexLeft: slicingIndexRight]).permute(1,2,0)
        

