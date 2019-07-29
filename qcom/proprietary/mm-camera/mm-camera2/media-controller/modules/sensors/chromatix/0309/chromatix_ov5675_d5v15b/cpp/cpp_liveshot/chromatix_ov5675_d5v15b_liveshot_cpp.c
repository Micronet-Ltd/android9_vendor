/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_cpp.h"

static chromatix_cpp_type chromatix_ov5675_d5v15b_parms = {
#if CAM_PRODUCT_C600
#include "chromatix_ov5675_d5v15b_c600_liveshot_cpp.h"
#elif CAM_PRODUCT_C800
#include "chromatix_ov5675_d5v15b_c800_liveshot_cpp.h"
#else
#include "chromatix_ov5675_d5v15b_liveshot_cpp.h"
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
