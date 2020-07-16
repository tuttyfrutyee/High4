# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import torch
import torch.nn.functional as F
import threading
from threading import Thread
import time
from threading import RLock
import numpy as np
import matplotlib.pyplot as plt

import Validation.RealTimeValidation.mqttStreamSink as MqttStream

dtype = torch.float


lock = RLock()

def realTimeEvaluateStream(net, stream, preHidden, threshold = 0.9):
    
    #print("streaming : " + str(stream["streaming"]))
    
    lastStreamIndex = -1    
    
    while(True):
        
        if(not(stream["streaming"]) and lastStreamIndex == len(stream["data"]) - 1):
            break        
        
        if(stream["streamIndex"] > lastStreamIndex):
            lastStreamIndex +=1
            

            
            with torch.no_grad():
                print(stream["data"][lastStreamIndex].shape)
                predict = net(stream["data"][lastStreamIndex]).permute(1,0,2)
                softMaxed = F.softmax(predict, dim=2)[0,0,:]
                #print(softMaxed)
                #print(F.softmax(predict,dim=2)[0,0,:].max(axis=0))
                index = torch.argmax(softMaxed)
                softmaxValue = softMaxed[index]
               
            with lock:
                if(softmaxValue > threshold):
                    stream["realTimePredicts"].append(index)

                else:
                    #print(softmaxValue)
                    #print(index)
                    stream["realTimePredicts"].append(0)
                    
                    
            
def evaluateMqttStreamRealTime(net, duration, windowSize, threshold):

    net.eval()
    net = net.cpu()
    hidden = (torch.zeros(1,1,net.hiddenSize, dtype=dtype), torch.zeros(1,1,net.hiddenSize, dtype=dtype))  
    net.hidden = hidden

    startTime = time.time()
    
    realTimePredicts = []
    
    for i in range(windowSize):
        realTimePredicts.append(0)    
        
    fig = plt.figure()
    ax = fig.add_subplot(111)
    line1, = ax.plot(realTimePredicts[-windowSize:], 'r-')
    plt.ylim(0,11)         
    
    fileMomentIndex = -1
    streamingData = []
    
    stream = {
                "streaming" : True,
                "data" : streamingData,
                "streamIndex" : fileMomentIndex,
                "realTimePredicts" : realTimePredicts 
            }
    
    streamEvaluater = Thread(target=realTimeEvaluateStream, args = (net, stream, windowSize, threshold))   
    mqttStream = Thread(target=MqttStream.mqttSinkStream, args = (stream,))
    streamEvaluater.start()
    mqttStream.start()
    
    while((time.time() - startTime) < duration):
        
            line1.set_ydata(realTimePredicts[-windowSize:])
            fig.canvas.draw()
            fig.canvas.flush_events()

    
    stream["streaming"] = False
    stream["mqttClient"].disconnect()
    streamEvaluater.join()
    return torch.cat(streamingData,0)
    

def simulateRealTimeFromFilesRandom(net, xTorch, fileNames, fileCount, windowSize, delay, threshold):

    net.eval()
    net = net.cpu()
    hidden = (torch.zeros(1,1,net.hiddenSize, dtype=dtype), torch.zeros(1,1,net.hiddenSize, dtype=dtype))
    net.hidden = hidden      
    
    
    randomFileIndexes = np.random.choice(xTorch.shape[1], size=fileCount)
    
    realTimePredicts = []
    
    for i in range(windowSize):
        realTimePredicts.append(0)    
        
    fig = plt.figure()
    ax = fig.add_subplot(111)
    line1, = ax.plot(realTimePredicts[-windowSize:], 'r-')
    plt.ylim(0,11)           
    

    for fileIndex in randomFileIndexes:
        
    
        fileMomentIndex = -1
        streamingData = []

        
        stream = {
                    "streaming" : True,
                    "data" : streamingData,
                    "streamIndex" : fileMomentIndex,
                    "realTimePredicts" : realTimePredicts 
                }
                
        
        streamEvaluater = Thread(target=realTimeEvaluateStream, args = (net, stream, windowSize, threshold))
        streamer = Thread(target=streamFile, args = (xTorch, fileIndex, stream, delay))
        
        print("Now streaming : "+ fileNames[fileIndex])
        
        streamEvaluater.start()
        streamer.start()
        
        while(stream["streaming"]):
            
            line1.set_ydata(realTimePredicts[-windowSize:])
            fig.canvas.draw()
            fig.canvas.flush_events()
            
        stream["streaming"] = False  
        
        streamer.join()
        streamEvaluater.join()



def streamFile(xTorch, streamIndex, stream, delay):
    
    #assumes xTroch has the shape : (seq_len, batch, outputsize)
    
    for moment in xTorch[:,streamIndex,:]:
        moment = moment.view(1, 1, moment.shape[0])
        with lock:
            stream["data"].append(moment)
            stream["streamIndex"] += 1
            
        time.sleep(delay)
    
    time.sleep(2)
    
    with lock:
        stream["streaming"] = False    
    
    

















