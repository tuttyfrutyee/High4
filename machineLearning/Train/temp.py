# -*- coding: utf-8 -*-


os.environ['CUDA_VISIBLE_DEVICES'] = '0'
dtype = torch.float
device = torch.device("cuda:0")

batchSize = 256
epochCount = 100000
trainRatio = 0.85



net = high4Net.High4Net().cuda()

classWeights = torch.from_numpy(np.array([0.1, 5,5, 5,5,5,5,5,5])).float().cuda()
criterion = nn.CrossEntropyLoss(weight=classWeights)
optimizer = optim.Adam(net.parameters(), lr = 1e-4)

seeds = [21,20,60,51,77,79,90,74]
seed2 = 14

xBag = []
labelBag = []


for i,seed in enumerate(seeds):
    x, labels, fileNames = high4Dataset.getHigh4Dataset(seed,seed2)
    
    if(i == 0):
        xBag = x[0:int(x.shape[0]*trainRatio)]
        xValidate = x[int(x.shape[0]*trainRatio):]
        labelBag = labels[0:int(labels.shape[0]*trainRatio)]
        labelsValidate = labels[int(labels.shape[0]*trainRatio):]
        fileNamesValidate = fileNames[int(fileNames.shape[0]*trainRatio):]
    else:
        xBag = np.concatenate((xBag,x[0:int(x.shape[0]*trainRatio)]),axis=0)
        labelBag = np.concatenate((labelBag,labels[0:int(labels.shape[0]*trainRatio)]),axis=0)

#shuffle time
seed = np.random.randint(0,100)
np.random.seed(seed)
np.random.shuffle(xBag) 
np.random.seed(seed)
np.random.shuffle(labelBag)
np.random.seed(seed)

xTrain = xBag
labelsTrain = labelBag

#reshaping
xTrainReshaped = np.swapaxes(xTrain,0,1)
xValidateReshaped = np.swapaxes(xValidate,0,1)

#make those torched
xTorchTrain = torch.from_numpy(xTrainReshaped).float().cuda()
xTorchValidate = torch.from_numpy(xValidateReshaped).float().cuda()

yTorchTrain = torch.from_numpy(labelsTrain-1).long().cuda()
yTorchValidate = torch.from_numpy(labelsValidate-1).long().cuda()


#create to hidden, since the last batch might not be divisible quite
lastHiddenSize = int(xBag.shape[0]*trainRatio) % batchSize
#for the divisible ones
hidden = (torch.zeros(1,batchSize,high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,batchSize,high4Net.hiddenSize, dtype=dtype).cuda())
#for the last one
if(lastHiddenSize != 0):
    hiddenLast = (torch.zeros(1,lastHiddenSize,high4Net.hiddenSize, dtype=dtype).cuda(), torch.zeros(1,lastHiddenSize,high4Net.hiddenSize, dtype=dtype).cuda())

trainLosses = []

batchIterationCount = int(int(xBag.shape[0]*trainRatio) / batchSize)
if(lastHiddenSize != 0):
    batchIterationCount += 1

net.train()
for epoch in range(epochCount):
    
    for batchIndex in range(batchIterationCount):
        
        if(lastHiddenSize != 0 and batchIndex == (batchIterationCount-1)):
            net.hidden = hiddenLast
            slicingIndexLeft = int(xBag.shape[0]*trainRatio) - lastHiddenSize
            slicingIndexRight = int(xBag.shape[0]*trainRatio)
            
        else:
            net.hidden = hidden
            slicingIndexLeft = batchIndex * batchSize
            slicingIndexRight = (batchIndex+1) * batchSize
            
        predicts = net(xTorchTrain[:,slicingIndexLeft : slicingIndexRight]).permute(1,2,0)
    
        loss = criterion(predicts, yTorchTrain[slicingIndexLeft : slicingIndexRight,:])
        
        optimizer.zero_grad()
        
        if(epoch%100 == 0 and batchIndex == 0):
            print(epoch, loss.item())
            calTrainPercentageCorrectness()
            calValidatePercentageCorrectness()
    
        if(batchIndex == 0):
            trainLosses.append(loss.item())
            
        loss.backward()
        optimizer.step()    


plt.plot(trainLosses[-300:])
plt.show()
