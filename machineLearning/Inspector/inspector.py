# -*- coding: utf-8 -*-
import numpy as np
import matplotlib.pyplot as plt
import time

def visualizeRecords(records, fileNames, numberOfVisualization, maxImuIndex=3):
    
    np.random.seed(int(time.time()))
    
    randomRecordIndexes = np.random.randint(low = 0, high= records.shape[0], size=numberOfVisualization)
    
    for i,recordIndex in enumerate(randomRecordIndexes):
        
        randomImuIndex = np.random.randint(low=0,high=maxImuIndex+1)
        
        
        fig, axs = plt.subplots(2,1, constrained_layout=True)
        
        
        accMagnitudes = []
        gyroMagnitudes = []
        
        for moment in records[recordIndex]:
            
            accStartIndex = randomImuIndex * 6
            gyroStartIndex = randomImuIndex * 6 + 3
            
            accMagnitudes.append( moment[accStartIndex] **2 + moment[accStartIndex+1]**2 + moment[accStartIndex+2]**2)
            gyroMagnitudes.append( moment[gyroStartIndex]**2 + moment[gyroStartIndex+1]**2 + moment[gyroStartIndex+2]**2)
            
        axs[0].plot(accMagnitudes)
        axs[0].set_title("Acc Magnitudes")
        
        axs[1].plot(gyroMagnitudes)
        axs[1].set_title("Gyro Magnitudes")
        
        fig.suptitle(fileNames[recordIndex] + " - imu:" + str(randomImuIndex))
        
        plt.show()