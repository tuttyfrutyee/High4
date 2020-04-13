# -*- coding: utf-8 -*-

import DataFetcher.dataFetcher as Fetcher
import Train.train as Train
import Models.models as Models

import torch
import torch.optim as optim

import matplotlib.pyplot as plt

import numpy as np

import Validation.StaticValidation.staticValidator as StaticValidator
import Validation.RealTimeValidation.realTimeValidator as RealTimeValidator




Fetcher.fetchAllData("/DataFetcher/Records")



data = Fetcher.getDataForFingers([0,1,2,3], 0.9, 6, 3)




losses = np.array([])
classWeights = torch.from_numpy(np.array([1,1,1,1,1,1,1,1,1,1])).float().cuda()
classWeights *= 3
epoch = 0

net = Models.createHigh4Model("high4Net_finger_0123_10",0).cuda()

modelGroup = {}
modelGroup["epoch"] = epoch
modelGroup["net"] = net
modelGroup["optimizer"] = optim.Adam(modelGroup["net"].parameters(), lr = 1e-2)
modelGroup["losses"] = losses
modelGroup["classWeights"] = classWeights


Train.train(modelGroup, data, 600000,60)




Models.saveModelGroup(modelGroup, "0123_10") #98.06

Models.loadModelGroup(modelGroup,"0123_10")



net = modelGroup["net"]




xValTorch = torch.from_numpy(np.swapaxes(data["xVal"], 0,1)).float()
yValTorch = torch.from_numpy(data["yVal"]).long()

RealTimeValidator.simulateRealTimeFromFilesRandom(net, xValTorch, data["valFileNames"], 3, 600, 0.002, 0.9)

#streamEvaluater.is_alive()

StaticValidator.listValidatePerformanceWithThreshold(net, xValTorch.cpu(), yValTorch.cpu(), data["valFileNames"], 0.9)


StaticValidator.inspectValidatePerformanceByFileName(net, xValTorch.cpu(), yValTorch.cpu(), data["valFileNames"], "D_367_3")


StaticValidator.calPercentageCorrectness(net, xValTorch.cpu(), yValTorch.cpu(), "vallMain")


#torch.cuda.empty_cache()
