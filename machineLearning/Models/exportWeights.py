import numpy as np
import os
import re
import json



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
            
            


b = np.zeros((24,))
for i in range(24):
    b[i] = 1           
            
            
            
c = np.array([-0.3176785111427307128906250, -0.5461816787719726562500000, -0.3566177785396575927734375, 0.0342995636165142059326172, -0.6997856497764587402343750, -0.0267401020973920822143555, 0.0898808240890502929687500, 0.0569288469851016998291016, -0.1572178006172180175781250, -0.0391438864171504974365234, -0.2241633087396621704101562, -0.4806402623653411865234375, -0.4568349123001098632812500, 0.1402828246355056762695312, -0.0311733502894639968872070, 0.1454916745424270629882812, 0.0252589359879493713378906, -0.0236617922782897949218750, -0.1621160358190536499023438, 0.0298680886626243591308594, -0.3733824193477630615234375, 0.1132227480411529541015625, -0.2709673345088958740234375, -0.0775397345423698425292969, -0.4370883703231811523437500, -0.0261108204722404479980469, 0.0949564725160598754882812, -0.0997112020850181579589844, 0.0168507210910320281982422, -0.1292393356561660766601562, -0.6047384738922119140625000, 0.2393993586301803588867188, -0.6307338476181030273437500, -0.4683153629302978515625000, -0.3149267733097076416015625, -0.0663732588291168212890625, 0.1540273576974868774414062, 0.0201895162463188171386719, -0.0253807008266448974609375, 0.0974669009447097778320312])            
            
            
            