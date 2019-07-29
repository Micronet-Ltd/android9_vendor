/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "gtm_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_gtm_test_vector_validation -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_gtm_tv_validate(void *in, void *op)
{
  uint32_t i, low_bits, high_bits;
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)op;

   for (i = 0; i < mod_in->gtm.size; i++){
     low_bits = mod_op->gtm.table[i] & 0x3FFFF;
     high_bits = mod_op->gtm.table[i] >> 18;
     mod_op->gtm.table[i] = ((uint64_t)high_bits << 32) | low_bits;
   }

  for (i = 0; i < mod_in->gtm.size; i++)
    VALIDATE_TST_LUT64(mod_in->gtm.table[i],
                     mod_op->gtm.table[i], 0, "gtm_y_ratio", i);

  return 0;
} /*vfe_gtm_test_vector_validation*/
