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

import Inspector.inspector as Inspector




#fetch necessary files
records, labels, fileNames = Fetcher.fetchAllData("/DataFetcher/Records")

Fetcher.records = records
Fetcher.labels = labels
Fetcher.fileNames = fileNames

data = Fetcher.getDataForFingers([0,1,2,3], 0.9, 6, 3)



#get some variables ready for modelsGroup
losses = np.array([])
classWeights = torch.from_numpy(np.array([1,1,1,1,1,1,1,1,1,1])).float().cuda()
classWeights *= 3
epoch = 0



#constructing modelsGroup
net = Models.createHigh4Model("high4Net_finger_0123_10",0).cuda()

modelGroup = {}
modelGroup["epoch"] = epoch
modelGroup["net"] = net
modelGroup["optimizer"] = optim.Adam(modelGroup["net"].parameters(), lr = 1e-2)
modelGroup["losses"] = losses
modelGroup["classWeights"] = classWeights
modelGroup["stopTraining"] = False

modelGroup["net"] = modelGroup["net"].cuda()


#train time
Train.train(modelGroup, data, 300,60)
    


#saving models and loading
Models.saveModelGroup(modelGroup, "0123_10") #98.06
Models.loadModelGroup(modelGroup,"0123_10")



net = modelGroup["net"]



#for validation get data ready
xValTorch = torch.from_numpy(np.swapaxes(data["xVal"], 0,1)).float()
yValTorch = torch.from_numpy(data["yVal"]).long()


#streamEvaluater.is_alive()

#evalutaters

#static evaluation
StaticValidator.listValidatePerformanceWithThreshold(net, xValTorch.cpu(), yValTorch.cpu(), data["valFileNames"], 0.9)

StaticValidator.inspectValidatePerformanceByFileName(net, xValTorch.cpu(), yValTorch.cpu(), data["valFileNames"], "D_125")

StaticValidator.calPercentageCorrectness(net, xValTorch.cpu(), yValTorch.cpu(), "vallMain")

#realtime evaluation
#from file
RealTimeValidator.simulateRealTimeFromFilesRandom(net, xValTorch, data["valFileNames"], 6, 600, 0.002, 0.9)

#from mqtt stream
RealTimeValidator.evaluateMqttStreamRealTime(net, 30, 600, 0.9)

#inspect data
Inspector.visualizeRecords(records, fileNames, 4)

#torch.cuda.empty_cache()
#torch.cuda.memory_allocated()





