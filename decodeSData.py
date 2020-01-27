previousByte : bytes
from struct import *

formatString = "<1h" #to parse c float32 (esp32 is little endian)

with open("SDATA.BIN", "rb") as f:
    byte_ = f.read(1)

    i = 1

    previousByte = byte_

    while byte_:
        i = i+1
        # Do stuff with byte.
        byte_ = f.read(1)
        if(i%2 == 0):
            value = unpack(formatString,  previousByte + byte_)
            print(value)

        previousByte = byte_