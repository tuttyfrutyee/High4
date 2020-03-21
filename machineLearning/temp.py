a = [23,34,23,55,4423,532,5,234,55,214,55,6,345,-1,2,2,2,2,52,6543,34,6,345,676,223,677,45,56,33]


fireSequenceIndex = 0
catchCount = 0 # this should be 3 in order to confirm firesquence
fireSequence = [2,2,2,2]
previousEncounterIndex = -1
while(fireSequenceIndex < len(a)):

    if(a[fireSequenceIndex] == fireSequence[catchCount]):

        if(((previousEncounterIndex + 1) == fireSequenceIndex) or catchCount == 0):
            previousEncounterIndex = fireSequenceIndex
            catchCount+= 1
            if(catchCount == len(fireSequence)):
                catchCount = 0
                print(fireSequenceIndex)
                print("vuhuu")
        else:
            catchCount = 0
    
    fireSequenceIndex+=1