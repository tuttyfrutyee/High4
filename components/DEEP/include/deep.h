#include "dl_lib_matrix3d.h"
#include "weights.h"



void forwardPass(dl_matrix3d_t* input, dl_matrix3d_t* hidden, dl_matrix3d_t** cellState, dl_matrix3d_t** workArea);

void printMatrix3d(dl_matrix3d_t* matrix);

void dumpMatrix3d(dl_matrix3d_t* matrix);

dl_matrix3d_t ** createForwardPassWorkArea();

void fillZerosIntoMatrix3d(dl_matrix3d_t* matrix);