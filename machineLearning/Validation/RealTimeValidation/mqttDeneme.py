# -*- coding: utf-8 -*-

import paho.mqtt.client as mqtt
from struct import *

import subprocess


msg = None

formatStringInt16 = "<"+ '1' +"b" #to parse c int16_t (esp32 is little endian)


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/high4/notification")


def on_message(client, userdata, msg_):
    global msg
    msg = msg_
    message = unpack(formatStringInt16, msg.payload)[0]
    
    if(message == 4):
        subprocess.call(["xdotool", "key",  "XF86AudioRaiseVolume"])
        subprocess.call(["xdotool", "key",  "XF86AudioRaiseVolume"])
        subprocess.call(["xdotool", "key",  "XF86AudioRaiseVolume"])
        subprocess.call(["xdotool", "key",  "XF86AudioRaiseVolume"])
        subprocess.call(["xdotool", "key",  "XF86AudioRaiseVolume"])
        subprocess.call(["xdotool", "key",  "XF86AudioRaiseVolume"])        
        
    elif(message == 5):
        subprocess.call(["xdotool", "key", "XF86AudioLowerVolume"])
        subprocess.call(["xdotool", "key", "XF86AudioLowerVolume"])
        subprocess.call(["xdotool", "key", "XF86AudioLowerVolume"])
        subprocess.call(["xdotool", "key", "XF86AudioLowerVolume"])
        subprocess.call(["xdotool", "key", "XF86AudioLowerVolume"])
        subprocess.call(["xdotool", "key", "XF86AudioLowerVolume"])          
        
    elif(message == 6):
        subprocess.call(["xdotool", "key",  "XF86AudioPlay"])
    elif(message == 7):
        subprocess.call(["xdotool", "key", "XF86AudioNext"])
    elif(message == 8):
        subprocess.call(["xdotool", "key","XF86AudioPrev"])
        subprocess.call(["xdotool", "key","XF86AudioPrev"])        
        subprocess.call(["xdotool", "key","XF86AudioPrev"])        
        
        
        
    print(message)
    
    


client = mqtt.Client(transport="websockets")
client.on_connect = on_connect
client.on_message = on_message


client.connect("192.168.1.233", 9001)
client.loop_forever()