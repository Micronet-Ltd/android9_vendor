LOCAL_PATH := $(call my-dir)
QCRIL_DIR := ${LOCAL_PATH}/../..

include $(CLEAR_VARS)

LOCAL_CFLAGS               += -Wall -Werror -DRIL_SHLIB $(qcril_cflags)
LOCAL_CXXFLAGS             += -std=c++14 $(qcril_cppflags)
LOCAL_CPPFLAGS             += -std=c++14 $(qcril_cppflags)
LOCAL_LDFLAGS              += $(qcril_ldflags)
ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE             += $(qcril_sanitize)
endif
LOCAL_SRC_FILES            += $(call all-cpp-files-under, src) $(call all-c-files-under, src)

LOCAL_MODULE               := qcrilAndroidModule
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_HEADER_LIBRARIES     += libril-qc-hal-qmi-headers libcutils_headers

include $(BUILD_STATIC_LIBRARY)
