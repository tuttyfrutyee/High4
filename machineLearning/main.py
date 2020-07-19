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


torch.nn.Module.dump_patches = True



#fetch necessary files
records, labels, fileNames = Fetcher.fetchAllData("/DataFetcher/Records")

Fetcher.records = records
Fetcher.labels = labels
Fetcher.fileNames = fileNames

data = Fetcher.getDataForFingers([0,1,2], 0.8, 6, 3)



#get some variables ready for modelsGroup
losses = np.array([])
classWeights = torch.from_numpy(np.array([1,1,1,1,1,1,1,1,1,1])).float().cuda()
classWeights *= 3
epoch = 0



#constructing modelsGroup
net = Models.createHigh4Model("high4Net_finger_0123_10",0).cuda()

net = Models.createHigh4Model("high4Net_finger_012_10",0).cuda()


modelGroup = {}
modelGroup["epoch"] = epoch
modelGroup["net"] = net
modelGroup["optimizer"] = optim.Adam(modelGroup["net"].parameters(), lr = 1e-2)
modelGroup["losses"] = losses
modelGroup["classWeights"] = classWeights
modelGroup["stopTraining"] = False

modelGroup["net"] = modelGroup["net"].cuda()


#train time
Train.train(modelGroup, data, 100,60)
    


#saving models and loading
Models.saveModelGroup(modelGroup, "0123_10") #98.06
Models.loadModelGroup(modelGroup,"0123_10")

Models.saveModelGroup(modelGroup, "012_10") #97.86
Models.loadModelGroup(modelGroup, "012_10")

net = modelGroup["net"].cpu()


#for validation get data ready
xValTorch = torch.from_numpy(np.swapaxes(data["xVal"], 0,1)).float()
yValTorch = torch.from_numpy(data["yVal"]).long()


#streamEvaluater.is_alive()

#evalutaters

#static evaluation
StaticValidator.listValidatePerformanceWithThreshold(net, xValTorch.cpu(), yValTorch.cpu(), data["valFileNames"], 0.9)

StaticValidator.inspectValidatePerformanceByFileName(net, xValTorch.cpu(), yValTorch.cpu(), data["valFileNames"], "D_223")

StaticValidator.calPercentageCorrectness(net, xValTorch.cpu(), yValTorch.cpu(), "vallMain")

#realtime evaluation
#from file
RealTimeValidator.simulateRealTimeFromFilesRandom(net, xValTorch, data["valFileNames"], 6, 600, 0.002, 0.9)

#from mqtt stream
record = RealTimeValidator.evaluateMqttStreamRealTime(net, 200, 600, 0.5)
record = (record - record.mean()) / record.std()
StaticValidator.predictRecord(net, record, threshold=0.1)
#inspect data
Inspector.visualizeRecords(records, fileNames, 4)
Inspector.visualizeRecords(record,["deneme"],1)

torch.cuda.empty_cache()
torch.cuda.memory_allocated()



Models.printNet(net)
parameters = Models.getParameters(net)




del net
net = modelGroup["net"].cpu()
dtype= torch.float

hidden = (torch.zeros(1,1,net.hiddenSize, dtype=dtype).cpu(), torch.zeros(1,1,net.hiddenSize, dtype=dtype).cpu())

net.hidden = hidden

trashInput = torch.ones((1,1,24))

for i in range(24):
    trashInput[0][0][i] = i+1

with torch.no_grad():
    
    for i in range(2):
        
        predicts = net(trashInput)
        print(predicts)

    

