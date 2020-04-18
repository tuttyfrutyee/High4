# -*- coding: utf-8 -*-

import paho.mqtt.client as mqtt
from struct import *
import time

import torch
import numpy as np

#normalising constants
accMean = -2.9482896854431004
accStd = 5.6099137739746405
gyroMean = -0.24911326810252488
gyroStd = 61.43354083464611

g = 9.807

numberOfImu = 4
linRange = 8
radRange = 1

formatStringInt16 = "<"+ str(numberOfImu * 6) +"h" #to parse c int16_t (esp32 is little endian)


stream = None

def on_message(client, userdata, msg):

    global mean
    global stream
    

        
    moment = unpack(formatStringInt16, msg.payload)  
    moment = np.array(moment).astype(float)
    
    for i, data in enumerate(moment):
        valueIndex = i % 6
        
        if(valueIndex < 3):
            moment[i] = (moment[i] / (65536.0 / 2.) * linRange * g)
        else:
            moment[i] = (moment[i] / (65536.0 / 2.) * radRange * 250 )     
            
#        if(valueIndex < 3):
#            moment[i] = ((moment[i] / (65536.0 / 2.) * linRange * g) - accMean) / accStd
#        else:
#            moment[i] = ((moment[i] / (65536.0 / 2.) * radRange * 250 ) - gyroMean) / gyroStd    

    
    torchMoment = torch.from_numpy(moment).view(1,1,len(moment)).float()
    
    stream["data"].append(torchMoment)
    stream["streamIndex"] += 1

def mqttSinkStream(stream_):
    
    global stream
    stream = stream_
    
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    
    stream_["mqttClient"] = client
    
    client.connect("192.168.1.233")
    client.loop_forever()
    

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/high4/stream")


    
