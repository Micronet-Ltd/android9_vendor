ifeq ($(BOARD_HAVE_BLUETOOTH_QCOM),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
ifeq ($(TARGET_HAS_SPLIT_A2DP_FEATURE),true)
ifeq ($(TARGET_USE_QTI_BT_STACK),true)
include $(LOCAL_PATH)/1.0/default/Android.mk
endif # TARGET_USE_QTI_BT_STACK
endif # TARGET_HAS_SPLIT_A2DP_FEATURE
endif # BOARD_HAVE_BLUETOOTH_QCOM
