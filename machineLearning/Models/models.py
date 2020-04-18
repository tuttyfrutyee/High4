# -*- coding: utf-8 -*-

import Models.nets as Nets
import torch

high4ModelsDict = {
        "high4Net_finger_0123_10" : [
                {   #ver0
                    "fingerCount" : 4,
                    "classSize" : 10,
                    "refinementSize" : 40,
                    "hiddenSize" : 40,
                },
        ],
        "high4Net_finger_012_10" : [
                {
                    "fingerCount" : 3,
                    "classSize" : 10,
                    "refinementSize" : 30,
                    "hiddenSize" : 30
                },
        ],
        "high4Net_finger_013_10" : [
                {
                    "fingerCount" : 3,
                    "classSize" : 10,
                    "refinementSize" : None,
                    "hiddenSize" : None
                },
        ],
        "high4Net_finger_02_10" : [
                {
                    "fingerCount" : 3,
                    "classSize" : 10,
                    "refinementSize" : None,
                    "hiddenSize" : None
                },
        ],                
                
      
}
        
def createHigh4Model(modelName, version):
    return Nets.High4Net(details = high4ModelsDict[modelName][version])


savePath = "Models/SavedModels/"

def saveModelGroup(modelGroup, saveName):
    
    saveDict = modelGroup
    
    saveDict["model_state_dict"] = modelGroup["net"].state_dict()
    saveDict["optimizer_state_dict"] = modelGroup["optimizer"].state_dict()
    
    torch.save(saveDict, savePath + saveName)

    

def loadModelGroup(modelGroup, saveName):
    
    loadDict = torch.load( savePath + saveName )
    
    for key in loadDict:
        if(not(key == "model_state_dict" or key == "optimizer_state_dict")):
            modelGroup[key] = loadDict[key]
        else:
            if(key == "model_state_dict"):
                modelGroup["net"].load_state_dict(loadDict[key])
            elif(key == "optimizer_state_dict"):
                modelGroup["optimizer"].load_state_dict(loadDict[key])
        

def printNet(net):
    for name, param in net.named_parameters():
        if param.requires_grad:
            print(name, param.data.shape)    
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            
            