/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "qdp_adapter.h"
#include "qdp.h"

#define QCRIL_DATA_IP_FAMILY_V4V6 "IPV4V6"

#define QCRIL_DATA_TECHNOLOGY_MAX_LEN  4
#define QCRIL_DATA_AUTH_PREF_MAX_LEN   2
#define QCRIL_PROFILE_ID_STR_MAX       4
#define QCRIL_IP_FAMILY_STR_MAX        sizeof(QCRIL_DATA_IP_FAMILY_V4V6)
#define QCRIL_APN_TYPES_STR_MAX        16
#define QCRIL_ROAMING_ALLOWED_STR_MAX  4

void dataprofile_get_legacy_params
(
   const BaseProfile::params_t in_params,
   char ** out_params
)
{
   out_params[QDP_RIL_TECH] = (char*)malloc(QCRIL_DATA_TECHNOLOGY_MAX_LEN + 1);
   strlcpy(out_params[QDP_RIL_TECH], std::to_string(in_params.radioTech).c_str(), QCRIL_DATA_TECHNOLOGY_MAX_LEN + 1);

   out_params[QDP_RIL_PROFILE_ID] = (char*)malloc(QCRIL_PROFILE_ID_STR_MAX + 1);
   strlcpy(out_params[QDP_RIL_PROFILE_ID], std::to_string(in_params.profileId).c_str(), QCRIL_PROFILE_ID_STR_MAX + 1);

   out_params[QDP_RIL_APN] = (char*)malloc(QMI_WDS_APN_NAME_MAX_V01 + 1);
   strlcpy(out_params[QDP_RIL_APN], in_params.apn.c_str(), QMI_WDS_APN_NAME_MAX_V01 + 1);

   out_params[QDP_RIL_NAI] = (char*)malloc(QMI_WDS_USER_NAME_MAX_V01 + 1);
   strlcpy(out_params[QDP_RIL_NAI], in_params.username.c_str(), QMI_WDS_USER_NAME_MAX_V01 + 1);

   out_params[QDP_RIL_PASSWORD] = (char*)malloc(QMI_WDS_PASSWORD_MAX_V01 + 1);
   strlcpy(out_params[QDP_RIL_PASSWORD], in_params.password.c_str(), QMI_WDS_PASSWORD_MAX_V01 + 1);

   out_params[QDP_RIL_AUTH] = (char*)malloc(QCRIL_DATA_AUTH_PREF_MAX_LEN + 1);
   strlcpy(out_params[QDP_RIL_AUTH], in_params.authType.c_str(), QCRIL_DATA_AUTH_PREF_MAX_LEN + 1);

   out_params[QDP_RIL_IP_FAMILY] = (char*)malloc(QCRIL_IP_FAMILY_STR_MAX + 1);
   strlcpy(out_params[QDP_RIL_IP_FAMILY], in_params.protocol.c_str(), QCRIL_IP_FAMILY_STR_MAX + 1);

   out_params[QDP_RIL_IP_ROAMING] = (char*)malloc(QCRIL_IP_FAMILY_STR_MAX + 1);
   strlcpy(out_params[QDP_RIL_IP_ROAMING], in_params.roamingProtocol.c_str(), QCRIL_IP_FAMILY_STR_MAX + 1);
};

void dataprofile_release_legacy_params
(
  char ** out_params
)
{
  free(out_params[QDP_RIL_TECH]);
  out_params[QDP_RIL_TECH] = NULL;

  free(out_params[QDP_RIL_PROFILE_ID]);
  out_params[QDP_RIL_PROFILE_ID] = NULL;

  free(out_params[QDP_RIL_APN]);
  out_params[QDP_RIL_APN] = NULL;

  free(out_params[QDP_RIL_NAI]);
  out_params[QDP_RIL_NAI] = NULL;

  free(out_params[QDP_RIL_PASSWORD]);
  out_params[QDP_RIL_PASSWORD] = NULL;

  free(out_params[QDP_RIL_AUTH]);
  out_params[QDP_RIL_AUTH] = NULL;

  free(out_params[QDP_RIL_IP_FAMILY]);
  out_params[QDP_RIL_IP_FAMILY] = NULL;

  free(out_params[QDP_RIL_IP_ROAMING]);
  out_params[QDP_RIL_IP_ROAMING] = NULL;
};
