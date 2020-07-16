#include "deep.h"



void fillZerosIntoMatrix3d(dl_matrix3d_t* matrix){
    for(int n = 0; n < matrix->n; n++){
        for(int h = 0; h < matrix->h; h++){
            for(int w = 0; w < matrix->w; w++){
                for(int c = 0; c < matrix->c; c++){
                    matrix->item[n * (matrix->h * matrix->w) + h * matrix->stride + w*matrix->c + c] = 0;
                }
            }
        }
    }    
}

void printMatrix3d(dl_matrix3d_t* matrix){
    printf("\n****************\n");
    for(int n = 0; n < matrix->n; n++){
        for(int h = 0; h < matrix->h; h++){
            
            for(int w = 0; w < matrix->w; w++){
                
                for(int c = 0; c < matrix->c; c++){
                    
                    printf("%.6f, ",matrix->item[n * (matrix->h * matrix->stride) + h * matrix->stride + w*matrix->c + c]);
                }
                printf("\n");
            }
            printf("\n\n");
        }
        printf("\n\n");
    }
    printf("\n****************\n");

}

void dumpMatrix3d(dl_matrix3d_t* matrix){
    printf("\n****************\n");

    for(int i = 0; i < (matrix->n * matrix->h * matrix->w * matrix->c); i++)
        printf("%.6f, ", matrix->item[i]);

    printf("\n****************\n");
}


void tanhMatrix3d(dl_matrix3d_t* matrix){
    for(int n = 0; n < matrix->n; n++){
        for(int h = 0; h < matrix->h; h++){
            for(int w = 0; w < matrix->w; w++){
                for(int c = 0; c < matrix->c; c++){
                    int index = n * (matrix->h * matrix->stride) + h * matrix->stride + w*matrix->c + c;
                    matrix->item[index] = tanh(matrix->item[index]);
                }
            }
        }
    }    
}



dl_matrix3d_t* copyReturnMatrix(dl_matrix3d_t* matrix){
    dl_matrix3d_t* copyMatrix = (dl_matrix3d_t *) dl_matrix3d_alloc(matrix->n, matrix->w, matrix->h, matrix->c);
    for(int i = 0; i < matrix->n * matrix->h * matrix->w* matrix->c; i++){
        copyMatrix->item[i] = matrix->item[i];
    }
    return copyMatrix;
}


float sigmoid(float x)
{
     float exp_value;
     float return_value;

     /*** Exponential calculation ***/
     exp_value = exp((double) -x);

     /*** Final sigmoid value ***/
     return_value = 1 / (1 + exp_value);

     return return_value;
}


void sigmaMatrix3d(dl_matrix3d_t* matrix){
    for(int n = 0; n < matrix->n; n++){
        for(int h = 0; h < matrix->h; h++){
            for(int w = 0; w < matrix->w; w++){
                for(int c = 0; c < matrix->c; c++){
                    int index = n * (matrix->h * matrix->stride) + h * matrix->stride + w*matrix->c + c;
                    matrix->item[index] = sigmoid(matrix->item[index]);
                }
            }
        }
    }    
}

void freeMatrixContent(dl_matrix3d_t * matrix){
    dl_lib_free(matrix->item);
}

void softmax3dMatrix(dl_matrix3d_t * matrix){
    //first sum all
    float totalSum = 0.0;

    for(int c = 0; c < matrix->c; c++){
        for(int w = 0; w < matrix->w; w++){
            for(int h = 0; h < matrix->h; h++){
                for( int n = 0; n < matrix->n; n++){

                    totalSum += exp(matrix->item[n * (matrix->h * matrix->stride) + h * matrix->stride + w*c + c]);

                }
            }
        }
    }

    for(int c = 0; c < matrix->c; c++){
        for(int w = 0; w < matrix->w; w++){
            for(int h = 0; h < matrix->h; h++){
                for( int n = 0; n < matrix->n; n++){

                    matrix->item[n * (matrix->h * matrix->stride) + h * matrix->stride + w*c + c] = exp(matrix->item[n * (matrix->h * matrix->stride) + h * matrix->stride + w*c + c]) / totalSum;

                }
            }
        }
    }    

}


void innerProduct3dMatrix(dl_matrix3d_t* result, dl_matrix3d_t* matrix1, dl_matrix3d_t* matrix2){
    
    for(int n = 0; n < matrix1->n; n++){
        for(int h = 0; h < matrix1->h; h++){
            for(int w = 0; w < matrix1->w; w++){
                for(int c = 0; c < matrix1->c; c++){
                    int index = n * (matrix1->h * matrix1->stride) + h * matrix1->stride + w*matrix1->c + c;
                    result->item[index] = matrix1->item[index] * matrix2->item[index];
                }
            }
        }
    }    
}

dl_matrix3d_t ** createForwardPassWorkArea(){
    //intermediate outputs
    dl_matrix3d_t * refinedOut = (dl_matrix3d_t *)dl_matrix3d_alloc(1, 1, 40,1); 
    
    dl_matrix3d_t * outI = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 1, 160, 1);
    dl_matrix3d_t * outH = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 1, 160, 1);

    dl_matrix3d_t * outIt = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 40, 1, 1);
    dl_matrix3d_t * outFt = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 40, 1, 1);
    dl_matrix3d_t * outGt = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 40, 1, 1);
    dl_matrix3d_t * outOt = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 40, 1, 1);

    dl_matrix3d_t * temp1 = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 40, 1, 1);
    dl_matrix3d_t * temp2 = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 40, 1, 1);
    dl_matrix3d_t * temp3 = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 40, 1, 1);

    //final output
    dl_matrix3d_t * finalOutput = (dl_matrix3d_t *) dl_matrix3d_alloc(1, 1, 10, 1);

    freeMatrixContent(outIt);
    freeMatrixContent(outFt);
    freeMatrixContent(outGt);
    freeMatrixContent(outOt);    

    dl_matrix3d_t ** workArea = (dl_matrix3d_t ** ) malloc(sizeof(dl_matrix3d_t*) * 11);

    workArea[0] = refinedOut;
    workArea[1] = outI;
    workArea[2] = outH;

    workArea[3] = outIt;
    workArea[4] = outFt;
    workArea[5] = outGt;
    workArea[6] = outOt;
    
    workArea[7] = temp1;
    workArea[8] = temp2;
    workArea[9] = temp3;

    workArea[10] = finalOutput;

    return workArea;

}


void forwardPass(dl_matrix3d_t* input, dl_matrix3d_t* hidden, dl_matrix3d_t** cellState, dl_matrix3d_t** workArea){

        //intermediate outputs
    dl_matrix3d_t * refinedOut = workArea[0];
    
    dl_matrix3d_t * outI = workArea[1];
    dl_matrix3d_t * outH = workArea[2];

    dl_matrix3d_t * outIt = workArea[3];
    dl_matrix3d_t * outFt = workArea[4];
    dl_matrix3d_t * outGt = workArea[5];
    dl_matrix3d_t * outOt = workArea[6];

    dl_matrix3d_t * temp1 = workArea[7];
    dl_matrix3d_t * temp2 = workArea[8];
    dl_matrix3d_t * temp3 = workArea[9];

    //final output
    dl_matrix3d_t * finalOutput = workArea[10];

    dl_matrix3dff_fc_with_bias(refinedOut, input, &refinementLayerweight, &refinementLayerbias);

    dl_matrix3dff_fc_with_bias(outI, refinedOut, &lstmweight_ih_l0, &lstmbias_ih_l0); 
    dl_matrix3dff_fc_with_bias(outH, hidden, &lstmweight_hh_l0, &lstmbias_hh_l0);

    dl_matrix3d_t * outSum = dl_matrix3d_add(outI, outH);


    outIt->item = outSum->item;
    outFt->item = outSum->item + 40;
    outGt->item = outSum->item + 80;
    outOt->item = outSum->item + 120;

    sigmaMatrix3d(outIt);
    sigmaMatrix3d(outFt);
    tanhMatrix3d(outGt);
    sigmaMatrix3d(outOt);


    innerProduct3dMatrix(temp1, outFt, *cellState);
    innerProduct3dMatrix(temp2, outIt, outGt);


    dl_matrix3d_free(*cellState);
    *cellState = dl_matrix3d_add(temp1, temp2);


    dl_matrix3d_t * tanhed = copyReturnMatrix(*cellState);
    tanhMatrix3d(tanhed);



    innerProduct3dMatrix(hidden, outOt, tanhed);

    dl_matrix3d_free(tanhed);
    dl_matrix3d_free(outSum);




    dl_matrix3dff_fc_with_bias(finalOutput, hidden, &layerFinalweight, &layerFinalbias);
    finalOutput->n = 1;
    finalOutput->w = 1;
    finalOutput->h = 1;
    finalOutput->c = 10;

    softmax3dMatrix(finalOutput);    

}
