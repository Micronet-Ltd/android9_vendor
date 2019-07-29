## BoardConfigVendor.mk
## Qualcomm Technologies proprietary product specific compile-time definitions.
#
USE_CAMERA_STUB := true

##SECIMAGE tool feature flags
USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN := 1
USES_SEC_POLICY_INTEGRITY_CHECK := 1
USE_SOC_HW_VERSION := true
SOC_HW_VERSION := 0x60080100  # Needs update for Kona
SOC_VERS := 0x6008  # Needs update for Kona
#Flags for generating signed images
USESECIMAGETOOL := true
QTI_GENSECIMAGE_MSM_IDS := kona  # Needs update for Kona
QTI_GENSECIMAGE_SIGNED_DEFAULT := kona  # Needs update for Kona
#USES_SEC_POLICY_MULTIPLE_DEFAULT_SIGN := 1
#USES_SEC_POLICY_INTEGRITY_CHECK := 1
