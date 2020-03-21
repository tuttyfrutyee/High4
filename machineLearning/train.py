#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Mar  6 12:12:52 2020

@author: tuttyfrutyee
"""
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
import os

import matplotlib.pyplot as plt

import numpy as np

import high4Dataset

os.environ['CUDA_VISIBLE_DEVICES'] = '0'
#
dtype = torch.float
device = torch.device("cuda:0")

batchSize = 63  

class Net(nn.Module):
    def __init__(self):
        super(Net, self).__init__()
        self.lstm = nn.LSTM(24,15).float()
        self.layerFinal = nn.Linear(15,9).float()
        self.hidden = (torch.zeros(1,batchSize,15, dtype=dtype).cuda(), torch.zeros(1,batchSize,15, dtype=dtype).cuda())
    def forward(self, x):
        x, hidden = self.lstm(x, self.hidden)
        x = self.layerFinal(F.relu(x))
        return x
        



net = Net()
net.cuda()



criterion = nn.CrossEntropyLoss()
optimizer = optim.Adam(net.parameters(), lr = 2e-5)

seed = 4
x, labels = high4Dataset.getHigh4Dataset(seed)

xReshaped = np.swapaxes(x,0,1)

xTorch = torch.from_numpy(xReshaped).float().cuda()
#yTorch = torch.from_numpy(labels.T - 1).long().unsqueeze(2)
#yTorch = torch.zeros(yTorch.shape[0], yTorch.shape[1], int(labels.max())).scatter_(2,yTorch,1)
yTorch = torch.from_numpy(labels-1).long().cuda()


losses = []

for i in range(100000):
    
    
    predicts = net(xTorch).permute(1,2,0)

    loss = criterion(predicts, yTorch)
    
    optimizer.zero_grad()
    
    if(i%100 == 0):
        print(i, loss.item())

    if(i%20 == 0):
        losses.append(loss.item())
        
    loss.backward()
    optimizer.step()


