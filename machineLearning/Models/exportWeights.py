import numpy as np
import os
import re
import json

import torch

def convert_3d_fc_w(data):
    (W, H) = data.shape
    fc_data = data.copy()
    return fc_data.reshape([1, W, H, 1])


def printFlatten(array):
    
    for d in array.flat:
        print(d, ",")



def exportFullConnectedWeightsToHeaderFile(weights_, fileName):
    
    with open("./Models/output/" + fileName + ".h", mode='w', encoding='utf-8') as fc:
        hdr = '#pragma once\n'
        hdr += '#include "dl_lib_matrix3d.h"\n'
        hdr += '#include "dl_lib_matrix3dq.h"\n\n'
        fc.writelines(hdr)    
    
    weights = dict(weights_)
    
    for key in weights:
        
        weightName = key.replace(".","")
        
        if(len(weights[key].shape) == 1):
            weights[key] = weights[key].reshape((weights[key].shape[0], 1)) 
        
        weight = convert_3d_fc_w(weights[key])
        
        item_f = "const static "
        
        item_f += f"fptp_t {weightName}_item_array[] = "
        data_template = "{:.8f}, "
        
        item_f += "{\n\t"
        intend = 0     
        
            
        for d in weight.flat:
            item_f += data_template.format(d)
            intend += 1
            if intend % 8 == 0:
                item_f += "\n\t"
        item_f += "\n};\n\n"       
            
        (N, H, W, C) = weight.shape
        struct_f = "const static dl_matrix3d"
        struct_f += f"_t {weightName} = {{\n" 
        struct_f += f"\t.w = {W},\n"
        struct_f += f"\t.h = {H},\n"
        struct_f += f"\t.c = {C},\n"
        struct_f += f"\t.n = {N},\n"
        struct_f += f"\t.stride = {W * C},\n"    
        
        struct_f += f"\t.item = ( fptp_t *)(&{weightName}_item_array[0])\n}};\n\n"        
            
        with open("./Models/output/" + fileName + ".h", mode='a', encoding='utf-8') as fc:
            fc.writelines(item_f)
            fc.writelines(struct_f)
            
            


x = np.array([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24])
hidden = np.zeros((40,))       

interMediate = np.dot(parameters["refinementLayer.weight"], x) + parameters["refinementLayer.bias"]

outI = np.dot(parameters["lstm.weight_ih_l0"], interMediate) + parameters["lstm.bias_ih_l0"]
outH = np.dot(parameters["lstm.weight_hh_l0"], hidden) + parameters["lstm.bias_hh_l0"]

out = torch.from_numpy(outI + outH)

cellState = torch.from_numpy(np.zeros((40,)))

it = torch.sigmoid(out[0:40])
ft = torch.sigmoid(out[40:80])
gt = torch.tanh(out[80:120])
ot = torch.sigmoid(out[120:160])

cellState = ft * cellState + it * gt
hidden = ot * torch.tanh(cellState)

finalLayer = np.dot(parameters["layerFinal.weight"], ht) + parameters["layerFinal.bias"]
output = torch.softmax(torch.from_numpy(finalLayer).float(), dim=0)

print(ht)            
print(ft*cellState)








a = np.array([3.351866, 
1.119191, 
-5.233476, 
-1.314588, 
1.682541, 
-3.483065, 
0.381004, 
0.285661, 
-3.440192, 
-3.505672])
    
a_torch = torch.from_numpy(a)
softA = np.array(torch.nn.Softmax(dim=0)(a_torch))


b = np.array([0.707322, 
0.077333, 
0.000128, 
0.006776, 
0.135424, 
0.000759, 
0.037044, 
0.033679, 
0.000793, 
0.000742])
    
c = np.array([0.711079, 


0.076257, 


0.000133, 


0.006688, 


0.133949, 


0.000765, 


0.036449, 


0.033135, 


0.000798, 


0.000748])









            
            
            