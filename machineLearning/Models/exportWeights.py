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
            
  