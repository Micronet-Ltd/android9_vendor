LOCAL_PATH := $(call my-dir)
PREBUILT_DIR_PATH := $(LOCAL_PATH)

ifeq ($(call is-board-platform,msm8937),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)),_32)
  -include $(LOCAL_PATH)/target/product/msm8937_32/Android.mk
endif
endif

ifeq ($(call is-board-platform,msm8937),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)),_32go)
  -include $(LOCAL_PATH)/target/product/msm8937_32go/Android.mk
endif
endif

ifeq ($(call is-board-platform,msm8937),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)),_64)
  -include $(LOCAL_PATH)/target/product/msm8937_64/Android.mk
endif
endif

ifeq ($(call is-board-platform,msm8953),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)),_32)
  -include $(LOCAL_PATH)/target/product/msm8953_32/Android.mk
endif
endif

ifeq ($(call is-board-platform,msm8953),true)
ifeq ($(strip $(TARGET_BOARD_SUFFIX)),_64)
  -include $(LOCAL_PATH)/target/product/msm8953_64/Android.mk
endif
endif

ifeq ($(call is-board-platform,msm8996),true)
  -include $(LOCAL_PATH)/target/product/msm8996/Android.mk
endif

-include $(sort $(wildcard $(PREBUILT_DIR_PATH)/*/*/Android.mk))
