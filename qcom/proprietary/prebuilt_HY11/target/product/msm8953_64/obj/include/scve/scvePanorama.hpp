/**=============================================================================

@file
scvePanorama.hpp

@brief
SCVE API Definition for Panorama feature.

Copyright (c) 2014-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

=============================================================================**/

//=============================================================================
///@defgroup scvePanorama
///@brief Defines API for SCVE-Panorama feature
///@ingroup scve
//=============================================================================

#ifndef SCVE_PANORAMA_HPP
#define SCVE_PANORAMA_HPP

#include "scveTypes.hpp"
#include "scveContext.hpp"

namespace SCVE
{

//------------------------------------------------------------------------------
/// @brief
///    Status codes specific to Panorama
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
enum StatusCodes_Panorama
{
   /// Returned by Run functions if the session's 'Init' is not called
   /// Major error: algorithm initialization failed. NEED BREAK
   SCVE_PANORAMA_ERROR_SESSION_NOT_INITED        = SCVE_PANORAMA_ERROR + 1,

   /// Returned by Run_Async function if there are too many Run_Async calls
   /// that are still pending. NEED BREAK
   SCVE_PANORAMA_ERROR_QUEUE_FULL                = SCVE_PANORAMA_ERROR + 2,

   /// Returned by panorama due to maximum frame reached
   /// ASSISTANT INFO, GO TO NEXT STEP (PANORAMA RESULT IS READY)
   SCVE_PANORAMA_ERROR_MAXIMUM_FRAMES_REACHED    = SCVE_PANORAMA_ERROR + 3,

   /// Returned by panorama due to frame not overlapped, it might be caused by
   /// moving too fast.
   /// ASSISTANT INFO, GO TO NEXT STEP (PANORAMA RESULT IS READY)
   SCVE_PANORAMA_ERROR_FRAME_NO_OVERLAP          = SCVE_PANORAMA_ERROR + 4,

   /// Returned by panorama due to moving direction not consitent
   /// ASSISTANT INFO, CONTINUE
   SCVE_PANORAMA_ERROR_DIRECTION_NOT_CONSITENT   = SCVE_PANORAMA_ERROR + 5,

   /// Returned by get result block due to result not ready
   /// ASSISTANT INFO, CONTINUE
   SCVE_PANORAMA_ERROR_RESULT_NOT_READY          = SCVE_PANORAMA_ERROR + 6,

   /// Returned by panorama due to no feature found in frame
   /// ASSISTANT INFO, CONTINUE
   SCVE_PANORAMA_ERROR_FRAME_NO_FEATURE          = SCVE_PANORAMA_ERROR + 7,

};

//------------------------------------------------------------------------------
/// @brief
///    Used for Panorama additional information along with frame data for
///    stitching (both through Sync and Async varieties)
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
enum FlagCodes_Panorama
{
   /// It would cancel current session and reset internal state.
   SCVE_PANORAMA_TASK_CANCEL                     = 0x0001,
   /// It would trigger panorama stitching process and panorama result would be
   /// generated after receving this flag through feedKeyFrame interface.
   SCVE_PANORAMA_FRAME_ENDOFSEQUENCE             = 0x0002,
};

//------------------------------------------------------------------------------
/// @brief
///    Panorama tracking detects motion direction and used for stitching
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
enum MotionDirections_Panorama
{
    SCVE_PANORAMA_MOTION_HORIZONTAL             = 0,
    SCVE_PANORAMA_MOTION_VERTICAL               = 1,
    SCVE_PANORAMA_MOTION_UNKNOWN                = -1,
};

//------------------------------------------------------------------------------
/// @brief
///    Panorama tracking configuration structure
///
/// @param iViewFinderDim
///    PanoramaTracking input frame Dimension for view finder
/// @param cFormat
///    PanoramaTracking input frame color format
/// @param fCameraFOV
///    Camera Field Of View. When 0, would use default value in algorithm.
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef struct TrackingConfiguration {
    ImageDimension  iViewFinderDim;
    ColorFormat     cFormat;
    float           fCameraFOV;

    INLINE TrackingConfiguration() : cFormat((ColorFormat)0), fCameraFOV(0.f) { }
}TrackingConfiguration_t;

//------------------------------------------------------------------------------
/// @brief
///    Panorama tracking input structure
///
/// @param pImage
///    source frame for Panorama.
/// @param rotation
///    The rotation angle in degrees. Rotation can only be 0, 90, 180 or 270.
/// @param flags
///    addtional information combined with source frame.
///    if SCVE_PANORAMA_TASK_CANCEL is set, it would cancel the task;
///    if SCVE_PANORAMA_FRAME_ENDOFSEQUENCE is set, the input source frame is
///        the last one. PanoramaTracking would not accept any input frame
///        more.
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef struct TrackingInput {
    Image         *pFrame;
    int32_t        rotation;
    int32_t        flags;

    INLINE TrackingInput() : pFrame(NULL), rotation(0), flags(0) { }
}TrackingInput_t;

//------------------------------------------------------------------------------
/// @brief
///    Panorama tracking output structure
///
/// @param isKeyFrame
///    Represents current frame is key frame or not
/// @param X
///    Horizontal movement distance of the frame, compared with first frame.
///    It is 0 for first frame.
/// @param Y
///    Vertical movement distance of the frame, compared with first frame.
///    It is 0 for first frame.
/// @param speed
///    Movement speed calculated by panorama tracking.
/// @param direction
///    The phone motion direction detected by panorama tracking.
///    It is -1 for unknown direction.
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef struct TrackingOutput {
   int32_t          isKeyFrame;
   int32_t          X;
   int32_t          Y;
   int32_t          speed;
   int32_t          direction;

   INLINE TrackingOutput() : isKeyFrame(0), X(0), Y(0), speed(0), direction((MotionDirections_Panorama)0) { }
}TrackingOutput_t;

//------------------------------------------------------------------------------
/// @brief
///    Definition of the Panorama's Tracking Callback function corresponding to
///    the PanoramaTracking::Run_Async function.
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef Status (*PanoramaTrackingCB)( Status status,
                                      TrackingOutput_t out,
                                      void* pSessionUserData,
                                      void* pTaskUserData );

//------------------------------------------------------------------------------
/// @brief
///    SCVE::PanoramaTracking class that implements SCVE's Panorama Tracking
///    feature.
///
/// @details
///    -
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
class SCVE_API PanoramaTracking
{
public:
   // @brief
   //     instantiate PanoramaTracking
   // @param config
   //     PanoramaTracking configuration
   // @param cbFunc
   //     callback function
   // @param pSessionUserData
   //     anything user want to preserve
   static PanoramaTracking* newInstance( Context* pScveContext,
                                         const TrackingConfiguration_t config,
                                         PanoramaTrackingCB cbFunc = NULL,
                                         void* pSessionUserData = NULL );

   static Status deleteInstance( PanoramaTracking *pInstance );

   // @brief
   //     Method of sync call. Feed view finder frame for camera position
   //     estamation.
   // @param in
   //     input structure reference
   // @param out
   //     output structure reference
   virtual Status Run_Sync( const TrackingInput_t in,
                                  TrackingOutput_t &out ) = 0;

   // @brief
   //     Method of async call.
   // @param in
   //     input structure reference
   virtual Status Run_Async( const TrackingInput_t in,
                             void* pTaskUserData = NULL ) = 0;

protected:
   // @brief
   // destructor
   virtual ~PanoramaTracking( );
};

//------------------------------------------------------------------------------
/// @brief
///    Used for Panorama KeyFrame type information along with key frame data for
///    stitching (both through Sync and Async varieties)
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
enum KeyFrameType_Panorama
{
   SCVE_PANORAMASTITCHING_KEYFRAME_RAW           = 0,
   SCVE_PANORAMASTITCHING_KEYFRAME_JPEG          = 1,
};

//------------------------------------------------------------------------------
/// @brief
///    Panorama Stitching Key Frame JPEG type data structure.
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef struct KeyFrameJPEG
{
   uint8_t *pData;
   uint32_t size;

   INLINE KeyFrameJPEG() : pData(NULL), size(0) { }
} KeyFrameJPEG_t;

//------------------------------------------------------------------------------
/// @brief
///    Panorama Stitching Key Frame data structure used in Panorama Stitching
///    interface. Supporting JPEG compressed bit stream type and NV21 raw data
///    type.
///
/// @param rotation
///    rotation is valid for both RAW type and JPEG type KeyFrame. For JPEG type
///    KeyFrame, Panorama would not read the orientation information from JPEG header.
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef struct KeyFrame
{
   union
   {
      Image *pKeyFrameRaw;
      KeyFrameJPEG_t *pKeyFrameJPEG;
   } u;

   int32_t type;
   int32_t rotation;

   INLINE KeyFrame() : type(0), rotation(0) { }
} KeyFrame_t;

//------------------------------------------------------------------------------
/// @brief
///    Projection type information for stitching
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
enum ProjectionType_Panorama
{
   SCVE_PANORAMASTITCHING_PROJECTION_SPHERICAL           = 0,
   SCVE_PANORAMASTITCHING_PROJECTION_CYLINDRICAL         = 1,
};

//------------------------------------------------------------------------------
/// @brief
///    Panorama stitching input structure
///
/// @param iViewFinderDim
///    The dimension used by view finder. For stitching input image
///    dimension, it uses the first image dimension feeding by run_sync or
///    run_async interface.
/// @param fCameraFocalLength
///    The camera focal length. When 0, would use default value in algorithm.
/// @param fCameraPixelSize
///    The camera pixel size (micrometer). When 0, would use default value
///    in algorithm.
/// @param iProjectionType
///    Projection type used for Panorama Stitching.
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef struct StitchingConfiguration {
    ImageDimension            iViewFinderDim;
    float                     fCameraFocalLength;
    float                     fCameraPixelSize;
    int32_t                   iProjectionType;

    INLINE StitchingConfiguration() : fCameraFocalLength(0.f), fCameraPixelSize(0.f), iProjectionType((ProjectionType_Panorama)0) { }
}StitchingConfiguration_t;

//------------------------------------------------------------------------------
/// @brief
///    Panorama stitching input structure
///
/// @param KeyFrame_t
///    Keyframe data.
/// @param X
///    The key frame X information which is got from tracking module.
/// @param Y
///    The key frame Y information which is got from tracking module.
/// @param flags
///    addtional information combined with source frame.
///    if SCVE_PANORAMA_TASK_CANCEL is set, it would cancel the task;
///    if SCVE_PANORAMA_FRAME_ENDOFSEQUENCE is set, the input source
///    frame is the last one
/// @param direction
///    The phone motion direction which is got from tracking module.
///    It is -1 for unknown direction.
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef struct StitchingInput {
    KeyFrame_t          *pKeyFrame;
    int32_t              X;
    int32_t              Y;
    int32_t              flags;
    int32_t              direction;

    INLINE StitchingInput() : pKeyFrame(NULL), X(0), Y(0), flags(0), direction((ProjectionType_Panorama)0) { }
}StitchingInput_t;

//------------------------------------------------------------------------------
/// @brief
///    Panorama stitching output structure
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef struct StitchingOutput {

   INLINE StitchingOutput() { }
}StitchingOutput_t;

//------------------------------------------------------------------------------
/// @brief
///    Definition of the Panorama's Stitching Callback function.
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
typedef Status (*PanoramaStitchingCB)( Status status,
                                       StitchingOutput_t out,
                                       void* pSessionUserData,
                                       void* pTaskUserData );

//------------------------------------------------------------------------------
/// @brief
///    SCVE::PanoramaStitching class that implements SCVE's Panorama Stitching
///    feature.
///
/// @details
///    -
///
/// @ingroup scvePanorama
//------------------------------------------------------------------------------
class SCVE_API PanoramaStitching
{
public:
   // @brief
   //     instantiate PanoramaStitching
   // @param config
   //     configuration structure reference
   // @param cbFunc
   //     PanoramaStitching result call back
   // @param pSessionUserData
   //     anything user want to preserve
   static PanoramaStitching* newInstance( Context* pScveContext,
                                          const StitchingConfiguration_t config,
                                          PanoramaStitchingCB cbFunc = NULL,
                                          void* pSessionUserData = NULL );

   static Status deleteInstance( PanoramaStitching *pInstance );

   // @brief
   //     Method of sync call. Feed keyframe data for Panorama Stitching.
   //     Panorama Stitching would take the first key frame size as the init
   //     size for this session.
   // @param in
   //     input structure reference
   // @param out
   //     output structure reference
   virtual Status Run_Sync( const StitchingInput_t in,
                                  StitchingOutput_t &out ) = 0;

   // @brief
   //     Method of async call. Feed keyframe data for Panorama Stitching.
   // @param in
   //     input structure reference
   virtual Status Run_Async( const StitchingInput_t in,
                             void* pTaskUserData = NULL ) = 0;

   // @brief
   //     Method of getting Panorama result.
   // @param pDst
   //     Panorama result bitsream (JPEG compressed) destination buffer.
   //     If pDst is NULL, pSize would return the whole panaroma image JPEG
   //     compressed bitstream size.
   // @param pSize
   //     Panorama result bitstream size.
   virtual Status GetPanoramaResult( uint8_t  *pDst,
                                     uint32_t *pSize ) = 0;

protected:
   // @brief
   // destructor
   virtual ~PanoramaStitching( );
};

} //namespace SCVE

#endif //SCVE_PANORAMA_HPP
