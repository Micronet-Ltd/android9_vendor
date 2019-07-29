/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_common.h"
#include "chromatix_3a.h"

static chromatix_3a_parms_type chromatix_ov5675_d5v15b_parms = {
#if CAM_PRODUCT_C600
#include "chromatix_ov5675_d5v15b_c600_default_video_3a.h"
#elif CAM_PRODUCT_C800
#include "chromatix_ov5675_d5v15b_c800_default_video_3a.h"
#else
#include "chromatix_ov5675_d5v15b_default_video_3a.h"
#endif
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
  return &chromatix_ov5675_d5v15b_parms;
}
