/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "BaseProfile.h"

void dataprofile_get_legacy_params(const BaseProfile::params_t in_params,
   char ** out_params);

void dataprofile_release_legacy_params(char ** out_params);

//TODO: ENABLE THIS. CURRENTLY NOT UNDER USE
//bool dataprofile_is_attach_required(const uint16_t profile_id);
