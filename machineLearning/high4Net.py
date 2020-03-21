#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Mar 21 09:44:30 2020

@author: tuttyfrutyee
"""

import torch
import torch.nn as nn
import torch.nn.functional as F


hiddenSize = 15
classSize = 9

class High4Net(nn.Module):
    def __init__(self):
        super(High4Net, self).__init__()
        self.lstm = nn.LSTM(24,hiddenSize).float()
        self.layerFinal = nn.Linear(hiddenSize,classSize).float()
        #self.hidden = (torch.zeros(1,batchSize,15, dtype=dtype).cuda(), torch.zeros(1,batchSize,15, dtype=dtype).cuda())
    def forward(self, x):
        x, hidden = self.lstm(x, self.hidden)
        x = self.layerFinal(F.relu(x))
        return x