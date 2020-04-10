# -*- coding: utf-8 -*-

def calTrainPercentageCorrectness():
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,batchSize,high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,batchSize,high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchTrain[:,0:batchSize,:]).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        equalityCount = (indicies == yTorchTrain[0:batchSize]).sum().item()
        totalCount = yTorchTrain.shape[1] * batchSize
        print("Train percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + " %")
    net.train()

def calValidatePercentageCorrectness():
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchValidate).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        equalityCount = (indicies == yTorchValidate).sum().item()
        totalCount = yTorchValidate.shape[0] * yTorchValidate.shape[1]
        print("Validate percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + " %")
    net.train()

def listValidatePerformances():
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchValidate).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        for i,indice in enumerate(indicies):
            equalityCount = (indice == yTorchValidate[i]).sum().item()
            totalCount =  yTorchValidate.shape[1]
            print("Validate percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + "% " + fileNamesValidate[i])
    net.train()

def inspectValidatePerformanceByFileName(fileName):
    targetIndex = -1
    for i,name in enumerate(fileNamesValidate):
        if(fileName in name):
            targetIndex = i
            break
    if(targetIndex is -1):
        print("File could not be found in validate files")
        return
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())
        net.hidden = hidden
        predicts = net(xTorchValidate).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        
        plt.plot(yTorchValidate[targetIndex].to("cpu"), "g", label="yTorchValidate")
        plt.plot(indicies[targetIndex].to("cpu"), "b", label="Predictions")
        plt.legend(loc="upper right")
        equality = (yTorchValidate[targetIndex] == indicies[targetIndex]).to("cpu")
#        for i,equal in enumerate(equality):
#            if(not equal):
#                plt.axvline(i,color="r")
        plt.show()
        
    net.train()

def listValidatePerformanceWithThreshold(threshold = 0.5):
    net.eval()
    with torch.no_grad():
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchValidate).cpu().permute(1,0,2)
        print(predicts.shape)
        values, indicies = predicts.max(axis=2)
        softmaxValues = F.softmax(predicts,dim=2).cpu()
        
        theValues = torch.zeros(indicies.shape[0], indicies.shape[1], dtype=torch.int64).cpu()
        
        for i,record in enumerate(indicies):
            for j,maxIndex in enumerate(record):
                if(softmaxValues[i][j][maxIndex] < threshold):
                    theValues[i][j] = 0
                else:
                    theValues[i][j] = maxIndex
                    
        validates = yTorchValidate.cpu()
        
        for i,record in enumerate(theValues):
            equalityCount = (record == validates[i]).sum().item()
            totalCount =  validates.shape[1]
            print("Validate percentage correctness: " + "%.2f" % (equalityCount / totalCount * 100) + "% " + fileNamesValidate[i])
    net.train()
    
def inspectValidatePerformanceByFileNameWithThreshold(fileName, threshold=0.5):
    print(threshold)
    targetIndex = -1
    for i,name in enumerate(fileNamesValidate):
        if(fileName in name):
            targetIndex = i
            break
    if(targetIndex is -1):
        print("File could not be found in validate files")
        return
    net.eval()
    with torch.no_grad():
       
        hidden = (torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,xValidate.shape[0],high4Net.hiddenSize, dtype=dtype).cuda())    
        net.hidden = hidden
        predicts = net(xTorchValidate).cpu().permute(1,0,2)
        values, maxIndicies = predicts.max(axis=2)
        softmaxValues = F.softmax(predicts,dim=2).cpu()
        theValues = torch.zeros(1, maxIndicies.shape[1], dtype=torch.int64).cpu()
        
        for j,maxIndex in enumerate(maxIndicies[targetIndex]):
            if(softmaxValues[targetIndex][j][maxIndex] < threshold):
                theValues[0][j] = 0
            else:
                theValues[0][j] = maxIndex
                
        plt.plot(yTorchValidate[targetIndex].to("cpu"), "g", label="yTorchValidate")
        plt.plot(theValues[0].to("cpu"), "b", label="Predictions")
        plt.legend(loc="upper right")

        plt.show()
        
    net.train()    
         