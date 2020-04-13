# -*- coding: utf-8 -*-

import torch
import torch.nn as nn
import torch.nn.functional as F



class High4Net(nn.Module):
    
    def __init__(self, details):
        
        super(High4Net, self).__init__()
        
        self.fingerCount = details["fingerCount"]
        self.refinementSize = details["refinementSize"]
        self.hiddenSize = details["hiddenSize"]
        self.classSize = details["classSize"]
        
        self.refinementLayer = nn.Linear(details["fingerCount"]* 6, details["refinementSize"]).float()
        self.lstm = nn.LSTM(details["refinementSize"],details["hiddenSize"]).float()
        self.layerFinal = nn.Linear(details["hiddenSize"],details["classSize"]).float()
        #self.hidden = (torch.zeros(1,batchSize,15, dtype=dtype).cuda(), torch.zeros(1,batchSize,15, dtype=dtype).cuda())
        
    def forward(self, x):
        
        x = self.refinementLayer(x)
        x, hidden = self.lstm(x, self.hidden)
        self.hidden = hidden
        x = self.layerFinal(x)
        return x
