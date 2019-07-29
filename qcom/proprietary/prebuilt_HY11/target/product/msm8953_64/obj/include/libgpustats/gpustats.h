/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef _GPUSTATS_H_
#define _GPUSTATS_H_

#ifdef __ANDROID__
#include <inttypes.h>
#include <arm_neon.h>
#include <stdlib.h>
#include <string.h>
#include "time.h"
#include "math.h"
#endif
#include <stdint.h>

#define GPU_MAX_SUPPORTED_WIDTH 4096
#define GPU_MAX_SUPPORTED_HEIGHT 2160
#define GPU_MAX_SUPPORTED_FPS 30
#define BIT(num) (1 << (num))

#define TILE_WIDTH_NV12_Y     32
#define TILE_HEIGHT_NV12_Y    8
#define TILE_WIDTH_NV12_UV    16
#define TILE_HEIGHT_NV12_UV   8


enum {
   GPU_PRIO_ERROR=0x1,
   GPU_PRIO_INFO=0x1,
   GPU_PRIO_HIGH=0x2,
   GPU_PRIO_LOW=0x4
};

#ifdef __ANDROID__
#include <cutils/properties.h>
#define PROPERTY_VALUE 92
extern int debug_level;

#undef GPUSTATS_PRINT_ERROR
#define GPUSTATS_PRINT_ERROR(fmt, args...) ({ \
                      if (debug_level & GPU_PRIO_ERROR) \
                          ALOGE(fmt,##args); \
                      })
#undef GPUSTATS_PRINT_INFO
#define GPUSTATS_PRINT_INFO(fmt, args...) ({ \
                      if (debug_level & GPU_PRIO_INFO) \
                          ALOGI(fmt,##args); \
                      })
#undef GPUSTATS_PRINT_LOW
#define GPUSTATS_PRINT_LOW(fmt, args...) ({ \
                      if (debug_level & GPU_PRIO_LOW) \
                          ALOGD(fmt,##args); \
                      })
#undef GPUSTATS_PRINT_HIGH
#define GPUSTATS_PRINT_HIGH(fmt, args...) ({ \
                      if (debug_level & GPU_PRIO_HIGH) \
                          ALOGD(fmt,##args); \
                      })
#else
#define GPUSTATS_PRINT_ERROR printf
#define GPUSTATS_PRINT_INFO printf
#define GPUSTATS_PRINT_LOW printf
#define GPUSTATS_PRINT_HIGH printf
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /**** Stats lib Status, to be returned by Stats lib ***/
    typedef enum gpu_stats_lib_op_status {
        SUCCESS,
        FAIL,
        UNUSED = 0x7FFFFFFF,
    } gpu_stats_lib_op_status;

    /**** Stats lib capabilities, Supported Algorithms Bitmask ***/
    typedef enum AlgoType {
        ADAPTIVE_QP,
        VQZIP_RT,
        MAX_SUPPORTED_ALGO,
    } AlgoType;

    /**** Stats lib capabilities, to be filled by Stats lib ***/
    typedef struct gpu_stats_lib_caps_t
    {
        uint32_t lib_version;
        uint32_t max_width;
        uint32_t max_height;
        uint32_t max_mb_per_sec;  // Always assumed that 1 MB = 16 X 16
        uint32_t color_formats;  //bitmask filled using enum color_fmts from msm_media_info.h
        uint32_t algo_types;  //bitmask
    } gpu_stats_lib_caps_t;

    /**** Stats lib performance hint. Will be decided by host ***/
    typedef enum perf_hint
    {
        LOW = 1,
        NORMAL = 2,
        HIGH = 3,
    } perf_hint;

	/***V2.0***/
	/*** Stas lib color compression format ***/
	 typedef enum color_compression_format
    {
        LINEAR_NV12 = 0,
        UBWC_NV12	= 1,
    } color_compression_format;

	//V2.0
    typedef struct ubwcYuvDataSize
    {
        uint32_t      nSize_Y_MetaData_Pitch;
        uint32_t      nSize_Y_MetaData;
        uint32_t      nSize_UV_MetaData_Pitch;
        uint32_t      nSize_UV_MetaData;
        uint32_t      nSize_Y_PixelData;
        uint32_t      nSize_UV_PixelData;
    } ubwcDataNV12;
    //V2.0

    typedef struct adaptive_qp_config_params_t
    {
        // roi_enabled is bool. But due to OMX this is made U32.
        uint32_t roi_enabled; //based on this flag, GPU should try to access roi_input
        // This corresponds to bIsRoiInfoAvailable in output buffer
        uint32_t pq_enabled; // if PQ is disabled, then GPU converts ROI data to PQ data.
        // This corresponds to bIsPqInfoAvailable in output buffer
        int32_t minDeltaQPlimit; // Final clipping range lower limit . With\without ROI
        int32_t maxDeltaQPlimit; // Final clipping range upper limit . With\without ROI
        float gain;
        float offset;
        uint32_t reserved[32];
    } adaptive_qp_config_params_t;

    typedef struct vqziprt_config_params_t
    {
        uint32_t reserved[64];
    } vqziprt_config_params_t;

    /**** Stats lib configuration, to be filled by video OMX component ***/
    typedef struct gpu_stats_lib_input_config
    {
        //uint32_t color_format;
        uint32_t algo;
        uint32_t width;
        uint32_t height;
        uint32_t crop_width;
        uint32_t crop_height;
        uint32_t mb_width;
        uint32_t mb_height;
        uint32_t stride;
        union { //union is based on flag algo type. Algo value is unique for session
            struct adaptive_qp_config_params_t a_qp;
            struct vqziprt_config_params_t vqzip_rt;
        };
    } gpu_stats_lib_input_config;

    /**** Stats lib buffer parameters, to be filled by video OMX component ***/
    typedef struct gpu_stats_lib_buffer_params_t
    {
        uint32_t fd;  // fd pointing to the buffer.
        uint32_t data_offset; // offset value from which the data in above buffer is valid.
        uint64_t flags; //not used currently
        uint32_t alloc_len; // Allocated length from starting buffer.
        // GPU should always accees between [0, alloc_len)
        // irrespective of data_offset.
        uint32_t filled_len; // Length of the valid data in buffer.
        // Data is valid only in [data_offset, filled_len)
        uint64_t timestamp; // not used currently
    } gpu_stats_lib_buffer_params_t;

    /**** ROI data layout ****/

    /**
     * int32_t  nUpperQpOffset;
     * int32_t  nLowerQpOffset;
     * uint32_t bUseRoiInfo;
     * int32_t  nRoiMBInfoSize;
     * void*    pRoiMBInfo;
     **/

    /**** ROI MB Info layout ****/

    /**
     * QP map buffer contains information about which offset should be applied to
     * which block on top of the QP calculated by hardware for that block. The
     * contents in QP buffer are stored linearly in memory (row-wise). Each cell
     * in the buffer contains 2-bits and represent a block in actual frame. The
     * interpretation of these 2 bits is as follows:

     * 2.b00 : keep current MB QP
     * 2.b01 : apply nLowerQpOffset
     * 2.b10 : apply nUpperQpOffset
     * 2.b11 : reserved
     **/

    /**** Output data layout ****/

    /**
     * uint32_t nQpOverrideInfoSize;   QP info array size.
     *                                 Size should be multiple of 16 bytes.
     *                                 8 bits for each MB/LCU.
     * uint32_t bIsPqInfoAvailable;    Boolean flag to indicate if PQ feature is enabled or not
     *
     * uint32_t bIsRoiInfoAvailable;   Boolean flag to indicate if ROI QP feature is enabled or not
     * int32_t  nAvgDeltaQp;           nAvgDeltaQp is calculated as follows in GPU library.
     *
     *                                 nAvgDeltaQp = Number of LCUs with lower QP offset * LowerQpOffset
     *                                               +
     *                                               Number of LCUs with upper QP offset * UpperQpOffset
     *                                 if(nAvgDeltaQp > 0)
     *                                       nAvgDeltaQp = (nAvgDeltaQp + numLcuFrame >>1) / numLcuFrame
     *                                 else
     *                                       nAvgDeltaQp = (nAvgDeltaQp - numLcuFrame >>1) / numLcuFrame
     * uint32_t reserved1;             Reserved word1 for future use
     * uint32_t reserved2;             Reserved word2 for future use
     * uint32_t pQpInfo[1];            Override QP buffer which contains delta QP values, and these delta QP
     *                                     values can be either positive or negative.
     *                                     H264: 8 bits are allocated for each MB(16x16) to code the deltaQP.
     *                                 HEVC: 8 bits are allocated for each LCU (32x32) to code the deltaQP.
     *                                 Firmware adds the delta QP to frameQP, and programs the resultant QP
     *                                 to hardware. If the final QP exceeds the [min, max] QP limits, then
     *                                 firmware clips the QP value to [min, max] QP limits, and programs
     *                                 the hardware.
     *
     *                                 If only PQ is enabled :
     *                                     pQpInfo buffer contains deltaQP values in the range [-6, 9] coded
     *                                     as a signed 8 bit value for every MB/LCU.
     *                                 If only ROI QP is enabled :
     *                                     pQpInfo buffer contains either nUpperQpOffset or nLowerQpOffset as per ROI,
     *                                     and is coded as a signed 8 bit value for every MB/LCU.
     *                                 If both PQ and ROI QP are enabled :
     *                                     pQpInfo buffer contains detaQP values calculated as
     *                                     deltaQP from PQ + nUpperQpOffset/nLowerQpOffset as per ROI.
     *                                     The resultant deltaQP value is coded as signed 8 bit value for
     *                                     every MB/LCU.
     *
     *                                 Note :
     *                                     The range of delta QP in ROI QP or ROI QP + PQ is [-16, 15].
     */

    /******************************************************/
    /** gpu_stats_lib_compile_prog
     *
     * Compile OpenCL code and write the generated binary to mem location.
     *
     * return: SUCCESS on success, FAIL when fails
     *         Host should check both Handle and return value for SUCCESS
     *
     * action:  SUCCESS : Continue
     *         FAIL : Nothing. Host will not do anything
     *
     **/

    gpu_stats_lib_op_status gpu_stats_lib_compile_prog(void **kernelMem, size_t *kernelSize, void** pGpu);

    /******************************************************/
    /** gpu_stats_lib_release_resource
      *  @pGpu: Reference to GPU object
      *
      *  Clean up the memory dynamically allocated memory.
      *
      **/
    void gpu_stats_lib_release_resource(void* pGpu);

    /******************************************************/
    /** gpu_stats_lib_init
     *    @lib_handle: gpu stats library handle
     *
     * Initializes gpu stats library and return handle to host.
     * Handle is unique for every instance.
     *
     * return: SUCCESS on success, FAIL when fails
     *         Host should check both Handle and return value for SUCCESS
     *
     * action: SUCCESS : Continue
     *         FAIL : Nothing. Host will not do anything
     *
     **/

    gpu_stats_lib_op_status gpu_stats_lib_init(void **handle, perf_hint perf, uint32_t color_form /*V2.0*/);

    /** gpu stats_lib_get_caps
     *    @handle: handle to gpu stats lib
     *    @caps: query capabilities data
     *
     * gpu stats lib fills it's capabilities. These capabilities may change run time.
     *
     * return: SUCCESS on success, FAIL when fails
     *
     * action: SUCCESS : Continue
     *         FAIL : Host will call deinit. gpu stats lib should clean-up internally.
     *
     **/
    gpu_stats_lib_op_status gpu_stats_lib_get_caps(void* handle, gpu_stats_lib_caps_t *caps);

    /** gpu stats_lib_configure
     *    @handle: handle to gpu stats lib
     *    @input_t: Configuration for current session.
     *
     * OMX component configures gpu stats lib
     *
     * return: SUCCESS on success, FAIL when fails
     *
     * action: SUCCESS : Continue
     *         FAIL : Host will call deinit. gpu stats lib should clean-up internally.
     *
     **/
    gpu_stats_lib_op_status gpu_stats_lib_configure(void* handle, gpu_stats_lib_input_config *input_t);

    /** gpu_stats_lib_fill_data
     *   @handle: handle to gpu_stats lib
     *   @yuv_input: pointer to input params consists input YUV
     *   Please refer YUV layouts in msm_media_info_h for a given color format.
     *   @roi_input: pointer to input params consists input ROI data.
     *   When ROI is not enabled in configure, this buffer can be NULL.
     *   @stats_output: pointer to output params consists Extradata buffer.
     *   roi_input and stats_output can point to same memory. GPU needs to inplace calcualtions.
     *    *   @ user_data: any client data to be passed back from gpu_stats_completion_callback
     *   @gpu_stats_completion_callback: function pointer to be called back upon GPU stats processing completion for non-blocking gpu_stats_lib_fill_data support
     setting gpu_stats_completion_callback to NULL will cause gpu_stats_lib_fill_data to block until GPU stats processing is complete.

     * main stats function to process YUV data and apply ROI on top of QP delta calculated by stats lib.
     *
     * return: SUCCESS on success, FAIL when fails
     *
     * action: SUCCESS : Continue
     *	       FAIL : Host can ignore this error and continue queueing next buffer.
     *         FAIL : Host will call deinit. Stats lib should clean-up internally.
     *
     **/

    gpu_stats_lib_op_status gpu_stats_lib_fill_data(void *handle,
            gpu_stats_lib_buffer_params_t *yuv_input,
            gpu_stats_lib_buffer_params_t *roi_input,
            gpu_stats_lib_buffer_params_t *stats_output,
            void (*gpu_stats_completion_callback) (void *handle, gpu_stats_lib_buffer_params_t *yuv_input,
                gpu_stats_lib_buffer_params_t *roi_input, gpu_stats_lib_buffer_params_t *stats_output, void *user_data),
            void *user_data
            );
    /** gpu_stats_lib_deinit
     *    @handle: gpu stats lib handle
     *
     * Deinitializes gpu stats module
     *
     * return: nothing
     **/
    void gpu_stats_lib_deinit(void *handle);

// Singleton class that builds OpenCL program in its constructor
// and keeps it in memory ready for use.
class GpuStatsBuilder {

    private:
    GpuStatsBuilder() : pGpu(NULL) {
        kernelSize = 0;
        pKernelMem = NULL;
        gpu_stats_lib_compile_prog(&pKernelMem, &kernelSize, &pGpu);
    }

    public:
    ~GpuStatsBuilder() {
        gpu_stats_lib_release_resource(pGpu);
    }
    static GpuStatsBuilder* getInstance() {
        static GpuStatsBuilder mInst;

        return &mInst;
    }

    void* getKernelMem() {
        return pKernelMem;
    }

    size_t getKernelSize() {
        return kernelSize;
    }

    private:
    void* pGpu;
    size_t kernelSize;
    void* pKernelMem;
};

#ifdef __cplusplus
}
#endif

#endif // _GPUSTATS_H_

