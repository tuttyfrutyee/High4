import os
from datetime import datetime


#first make sure that required directories and files does exists
filesAsString = os.listdir(".")

if(not("records" in filesAsString)):
    path = os.getcwd() + "/records"
    try:
        os.mkdir(path)
    except OSError: 
        print("Could not create records directory")

# now stack the files that has bin extension and d in it
sensorRecords = [s for s in filesAsString if any(xs in s for xs in ["BIN", "D"])]

#now create an array of the modes that sensorRecords has
modesRecorded = []
for record in sensorRecords:
    lastIndexOfSub = -1
    for i in range(len(record)):
        if(record[i] == "_"):
            lastIndexOfSub = i
        if(record[i] == "."):
            modesRecorded.append([record[lastIndexOfSub+1 : i], record])


#now make sure the directories corresponding to the modes exists
filesInRecords = os.listdir("./records")
for modeNumber in modesRecorded:
    if(not(modeNumber[0] in filesInRecords)):
        path = os.getcwd() + "/records/" + modeNumber[0]
        try:
            os.mkdir(path)
        except OSError:
            print("Could not mode directory : ",modeNumber[0])

#now move the sensorRecords to their corresponding directories
for record in modesRecorded:
    oldPath = os.getcwd() + "/"+ record[1]
    newPath = os.getcwd() + "/records/" + record[0] + "/" + record[1]
    os.replace(oldPath, newPath)

if(len(modesRecorded) == 0):
    print("Nothing to clear around, bye...")
    os._exit(1)

#now log all the changes to the recordLog.txt

f = open("./recordLog.txt","a+")
now = datetime.now()
tag = now.strftime("%d/%m/%Y, %H:%M:%S ---- ")

#write the date as tag
f.write(tag)

for i, record in enumerate(modesRecorded):
    if(i == 0):
        f.write(record[1])
    else:
        f.write(", " + record[1])

f.write("\n")

f.close()