LOCAL_PATH := $(call my-dir)
QCRIL_DIR := ${LOCAL_PATH}/../..
#
include $(CLEAR_VARS)

LOCAL_CFLAGS               += -Wall -Werror -DRIL_SHLIB $(qcril_cflags)
LOCAL_CXXFLAGS             += -std=c++14 $(qcril_cppflags)
LOCAL_CPPFLAGS             += -std=c++14 $(qcril_cppflags)
LOCAL_LDFLAGS			   += $(qcril_ldflags)
ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE             += $(qcril_sanitize)
endif
LOCAL_SRC_FILES            += $(call all-cpp-files-under, src)

LOCAL_MODULE               := qcrilSapModule
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_SHARED_LIBRARIES     += android.hardware.radio@1.0
LOCAL_SHARED_LIBRARIES     += android.hardware.radio@1.1
LOCAL_SHARED_LIBRARIES     += android.hardware.radio@1.2

LOCAL_HEADER_LIBRARIES     += libril-qc-hal-qmi-headers

include $(BUILD_STATIC_LIBRARY)
