LOCAL_PATH := $(call my-dir)
QCRIL_DIR := ${LOCAL_PATH}/..

## Build header library
include $(CLEAR_VARS)
LOCAL_MODULE               := libril-qc-hal-framework-headers
LOCAL_VENDOR_MODULE        := true
LOCAL_EXPORT_C_INCLUDE_DIRS += $(QCRIL_DIR)/include
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)

ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE			   := $(qcril_sanitize)
endif
LOCAL_CFLAGS               += -Wall -Werror -DRIL_SHLIB $(qcril_cflags)
LOCAL_CXXFLAGS             += -std=c++14
LOCAL_SRC_FILES            += $(call all-cpp-files-under, src)
LOCAL_SRC_FILES            += $(call all-c-files-under, src)
LOCAL_SRC_FILES            += $(call all-cc-files-under, src)

LOCAL_MODULE               := libqcrilFramework
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_HEADER_LIBRARIES     += libril-qc-hal-qmi-headers libcutils_headers libutils_headers

LOCAL_SHARED_LIBRARIES += libdiag \
     liblog \

LOCAL_SHARED_LIBRARIES += libsettings

include $(BUILD_SHARED_LIBRARY)
