# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt


def forwardHigh4Net(weights, x, hiddens):
    
    #hiddens[0] : h_t-1 , hiddens[1] : c_t-1
    
    refinementLW = weights["refinementLayer.weight"]
    refinementLB = weights["refinementLayer.bias"]
    
    lstmWIH = weights["lstm.weight_ih_l0"]
    lstmWHH = weights["lstm.weight_hh_l0"]
    
    lstmWIB = weights["lstm.bias_ih_l0"]
    lstmWHB = weights["lstm.bias_hh_l0"]
    
    finalLW = weights["layerFinal.weight"]
    finalLB = weights["layerFinal.bias"]
    
    #refinementLayer
    x = np.dot(refinementLW, x) + refinementLB
    
    #lstmLayer
    
    hiddenSize = int(lstmWIH.shape[0] / 4)
    
    lstmForwardInput = np.dot(lstmWIH, x) + lstmWIB
    lstmForwardHidden = np.dot(lstmWHH, hiddens[0]) + lstmWHB
    
    i_t = sigmoid( lstmForwardInput[0:hiddenSize] + lstmForwardHidden[0:hiddenSize] )
    f_t = sigmoid( lstmForwardInput[hiddenSize:hiddenSize*2] + lstmForwardHidden[hiddenSize:hiddenSize*2] )
    g_t = np.tanh( lstmForwardInput[hiddenSize*2:hiddenSize*3] + lstmForwardHidden[hiddenSize*2:hiddenSize*3] )
    o_t = sigmoid( lstmForwardInput[hiddenSize*3:hiddenSize*4] + lstmForwardHidden[hiddenSize*3:hiddenSize*4] )
    
    c_t = f_t * hiddens[1] + i_t * g_t
    h_t = o_t * np.tanh(c_t)
    
    #finalLayer
    x = np.dot(finalLW, h_t) + finalLB
    
    
def sigmoid(x):
    return 1 / (1+np.exp(-x))

def tanh(x):
    return ( np.exp(x) - np.exp(-x) ) / ( np.exp(x) + np.exp(-x) )


def inspectValidatePerformanceByFileName(weights, x, y, fileNames, targetFileName):
    
    net = net.cuda()
    
    targetIndex = -1
    for i,name in enumerate(fileNames):
        if(targetFileName in name):
            targetIndex = i
            break
    if(targetIndex is -1):
        print("File could not be found in validate files")
        return
    net.eval()
    
    windowSize = y.shape[0]

    
    with torch.no_grad():
        
        hidden = (torch.zeros(1,windowSize,net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,windowSize,net.hiddenSize, dtype=dtype).cuda())
        
        net.hidden = hidden
        
        predicts = net(x.cuda()).permute(1,2,0)
        values, indicies = predicts.max(axis=1)
        
        plt.figure()
        
        plt.plot(y[targetIndex].to("cpu"), "g", label="yTorchValidate")
        
        plt.plot(indicies[targetIndex].to("cpu"), "b", label="Predictions")
        
        plt.legend(loc="upper right")

        plt.show()
        
        del hidden
        del predicts
        del x 
        del y
        
        torch.cuda.empty_cache()
        
    net.train()
    