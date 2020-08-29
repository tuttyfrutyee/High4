# -*- coding: utf-8 -*-

import paho.mqtt.client as mqtt
from struct import *
import time
import numpy as np
import matplotlib.pyplot as plt

import threading
from threading import Thread
import time

expectedPackageCount = 0
packSize = 0

formatString_packageCount = "<"+ str(1) +"h" #to parse (esp32 is little endian)
formatString_packSize = "<"+ str(1) +"b" 

formatString_elements = "<"+ str(packSize) +"ffff" 

mode = 0
receivedPackageCount = 0

packages = []
plotFlag = 0


def plotPackages():
    
    global packages
    
    packages_ = np.array(packages)
    
    
    best3Indexes = np.flip((np.sum(packages_, axis=0) / packages_.shape[0]).argsort())[0:3]
    print(best3Indexes+1)
    
    plt.figure(0)
    plt.title("Best")
    plt.ylim(-0.1,1.1)
    plt.plot(packages_[:, best3Indexes[0]], linewidth=2)
    
    plt.figure(1)
    plt.title("Second")
    plt.ylim(-0.1,1.1)
    plt.plot(packages_[:, best3Indexes[1]], linewidth=2)
    
    plt.figure(2)
    plt.title("Third")
    plt.ylim(-0.1,1.1)
    plt.plot(packages_[:, best3Indexes[2]], linewidth=2)
    
def mqttMonitor():
    
    client = mqtt.Client(transport="websockets")
    client.on_connect = on_connect
    client.on_message = on_message
    
    
    client.connect("192.168.1.233", 9001)
    client.loop_forever()
            
            
def updateFormatString_elements():
    
    global formatString_elements
    
    formatString_elements = "<"+ str(packSize) +"f" 


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/high4/monitor")


def on_message(client, userdata, msg_):
    
     global expectedPackageCount
     global packSize
     
     global mode
     global receivedPackageCount
     
     global packages
     
     global msg
     
     global plotFlag
     
     msg = msg_
     
     

         
     if(mode == 0):
         packages = []
         expectedPackageCount = unpack(formatString_packageCount, msg.payload)[0]
         mode += 1
         
     elif(mode == 1):
         
         packSize = unpack(formatString_packSize, msg.payload)[0]       
         updateFormatString_elements()
         mode +=1
         
     elif(mode == 2):
         package = unpack(formatString_elements, msg.payload)
         packages.append(np.array(package))
         receivedPackageCount +=1
         
         if(receivedPackageCount == expectedPackageCount):
             mode = 0
             receivedPackageCount = 0
             print("got the packages")
             plotFlag = 1
            
        
    
    


monitor = Thread(target=mqttMonitor, args = ())

monitor.start()

"""
    monitor.join()
    
    
    plotPackages()
    
"""


#here plot functions




        
        
        

























