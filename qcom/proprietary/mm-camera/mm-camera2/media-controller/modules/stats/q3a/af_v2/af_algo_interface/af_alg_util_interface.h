/***************************************************************************
Copyright (c) 2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/
#ifndef __AF_ALG_UTIL_INTF_H__
#define __AF_ALG_UTIL_INTF_H__


/* =======================================================================

DEFINITIONS AND DECLARATIONS

This section contains definitions for constants, macros, types, variables
and other items needed by this interface.

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** af_alg_util_interface API
** ----------------------------------------------------------------------- */
typedef struct _af_alg_util_interface_t af_alg_util_interface_t;

struct _af_alg_util_interface_t {
    int          (*dist_2_pos)       (const af_alg_util_interface_t *p_alg_util, int distmm);
    unsigned int (*pos_2_dist)       (const af_alg_util_interface_t *p_alg_util, unsigned int lp);
    unsigned int (*dac_2_pos)        (const af_alg_util_interface_t *p_alg_util, int dac);
    int          (*pos_2_dac)        (const af_alg_util_interface_t *p_alg_util, unsigned int lp);
};

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
int dist_2_pos(const af_alg_util_interface_t *p_alg_util, int distmm);
unsigned int pos_2_dist(const af_alg_util_interface_t *p_alg_util, unsigned int lp);
unsigned int dac_2_pos(const af_alg_util_interface_t *p_alg_util, int dac);
int pos_2_dac(const af_alg_util_interface_t *p_alg_util, unsigned int lp);


#endif
