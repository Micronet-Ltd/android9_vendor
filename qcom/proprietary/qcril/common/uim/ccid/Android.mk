LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(call is-board-platform-in-list, msm8998 msmcobalt sdm845 sdm855),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi-framework/inc
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/qmi/inc

LOCAL_SRC_FILES:= \
   ccid_uim_daemon.c \
   ccid_uim_parsing.c

LOCAL_SHARED_LIBRARIES := libqmi_cci libqmi_encdec libqmiservices
LOCAL_HEADER_LIBRARIES += libril-qc-hal-qmi-headers

LOCAL_MODULE:= ccid_daemon
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)

endif

