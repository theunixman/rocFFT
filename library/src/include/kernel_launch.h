/*******************************************************************************
 * Copyright (C) 2016 Advanced Micro Devices, Inc. All rights reserved.
 ******************************************************************************/


#ifndef KERNEL_LAUNCH_SINGLE
#define KERNEL_LAUNCH_SINGLE

#define FN_PRFX(X) rocfft_internal_ ## X
#include <iostream>
#include "rocfft.h"
#include "rocfft_hip.h"
#include "tree_node.h"
#include "kargs.h"
#include "error.h"
#include "kernel_launch_generator.h"


struct DeviceCallIn
{
    TreeNode *node;
    void *bufIn[2];//TODO, bufIn[1] is never used
    void *bufOut[2];//TODO, bufOut[1] is never used

    GridParam gridParam;
};

struct DeviceCallOut
{
    int err;
};

extern "C"
{

    /* Naming convention 

    dfn – device function caller (just a prefix, though actually GPU kernel function)

    sp (dp) – single (double) precision

    ip – in-place

    op - out-of-place

    ci – complex-interleaved (format of input buffer)

    ci – complex-interleaved (format of output buffer)

    stoc – stockham fft kernel
    bcc - block column column 

    1(2) – one (two) dimension data from kernel viewpoint, but 2D may transform into 1D. e.g  64*128(2D) = 8192(1D)

    1024, 64_128 – length of fft on each dimension 

    */


    void rocfft_internal_transpose_var1_sp(const void *data_p, void *back_p);

    void rocfft_internal_transpose_var2(const void *data_p, void *back_p);
    
}

/*
   data->node->devKernArg : points to the internal length device pointer
   data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH : points to the intenal in stride device pointer 
   data->node->devKernArg + 2*KERN_ARGS_ARRAY_WIDTH : points to the internal out stride device pointer, only used in outof place kernels
*/

#define POWX_SMALL_GENERATOR(FUNCTION_NAME, IP_FWD_KERN_NAME, IP_BACK_KERN_NAME, OP_FWD_KERN_NAME, OP_BACK_KERN_NAME, PRECISION) \
void FUNCTION_NAME(const void *data_p, void *back_p)\
{\
    DeviceCallIn *data = (DeviceCallIn *)data_p;\
    if (data->node->placement == rocfft_placement_inplace) { \
        if(data->node->inStride[0] == 1 && data->node->outStride[0] == 1){ \
            if(data->node->direction == -1 ) {\
                hipLaunchKernel(HIP_KERNEL_NAME( IP_FWD_KERN_NAME<PRECISION, SB_UNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0]); \
            }\
            else{ \
                hipLaunchKernel(HIP_KERNEL_NAME( IP_BACK_KERN_NAME<PRECISION, SB_UNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0]); \
            }\
        } \
        else{ \
            if(data->node->direction == -1 ) {\
                hipLaunchKernel(HIP_KERNEL_NAME( IP_FWD_KERN_NAME<PRECISION, SB_NONUNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0]); \
            }\
            else{ \
                hipLaunchKernel(HIP_KERNEL_NAME( IP_BACK_KERN_NAME<PRECISION, SB_NONUNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0]); \
            }\
        } \
    }\
    else{ \
        if(data->node->inStride[0] == 1 && data->node->outStride[0] == 1){ \
            if(data->node->direction == -1) {\
                hipLaunchKernel(HIP_KERNEL_NAME( OP_FWD_KERN_NAME<PRECISION, SB_UNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, data->node->devKernArg + 2*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0], (PRECISION *)data->bufOut[0]); \
            }\
            else{ \
                hipLaunchKernel(HIP_KERNEL_NAME( OP_BACK_KERN_NAME<PRECISION, SB_UNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, data->node->devKernArg + 2*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0], (PRECISION *)data->bufOut[0]); \
            }\
        }\
        else{ \
            if(data->node->direction == -1) {\
                hipLaunchKernel(HIP_KERNEL_NAME( OP_FWD_KERN_NAME<PRECISION, SB_NONUNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, data->node->devKernArg + 2*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0], (PRECISION *)data->bufOut[0]); \
            }\
            else{ \
                hipLaunchKernel(HIP_KERNEL_NAME( OP_BACK_KERN_NAME<PRECISION, SB_NONUNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, data->node->devKernArg + 2*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0], (PRECISION *)data->bufOut[0]); \
            }\
        } \
    }\
}


#define POWX_LARGE_SBCC_GENERATOR(FUNCTION_NAME, FWD_KERN_NAME, BACK_KERN_NAME, PRECISION) \
void FUNCTION_NAME(const void *data_p, void *back_p)\
{\
    DeviceCallIn *data = (DeviceCallIn *)data_p;\
    if(data->node->direction == -1) {\
        hipLaunchKernel(HIP_KERNEL_NAME( FWD_KERN_NAME<PRECISION, SB_UNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, (PRECISION *)data->node->twiddles_large, \
                data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, data->node->devKernArg + 2*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0], (PRECISION *)data->bufOut[0]); \
    }\
    else{ \
        hipLaunchKernel(HIP_KERNEL_NAME( BACK_KERN_NAME<PRECISION, SB_UNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, (PRECISION *)data->node->twiddles_large, \
                data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, data->node->devKernArg + 2*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0], (PRECISION *)data->bufOut[0]); \
    }\
}

#define POWX_LARGE_SBRC_GENERATOR(FUNCTION_NAME, FWD_KERN_NAME, BACK_KERN_NAME, PRECISION) \
void FUNCTION_NAME(const void *data_p, void *back_p)\
{\
    DeviceCallIn *data = (DeviceCallIn *)data_p;\
    if(data->node->direction == -1) {\
        hipLaunchKernel(HIP_KERNEL_NAME( FWD_KERN_NAME<PRECISION, SB_UNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, \
                data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, data->node->devKernArg + 2*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0], (PRECISION *)data->bufOut[0]); \
    }\
    else{ \
        hipLaunchKernel(HIP_KERNEL_NAME( BACK_KERN_NAME<PRECISION, SB_UNIT> ), dim3(data->gridParam.b_x), dim3(data->gridParam.tpb_x), 0, 0, \
                (PRECISION *)data->node->twiddles, \
                data->node->length.size(), \
                data->node->devKernArg, data->node->devKernArg + 1*KERN_ARGS_ARRAY_WIDTH, data->node->devKernArg + 2*KERN_ARGS_ARRAY_WIDTH, \
                data->node->batch, (PRECISION *)data->bufIn[0], (PRECISION *)data->bufOut[0]); \
    }\
}


#endif // KERNEL_LAUNCH_SINGLE

