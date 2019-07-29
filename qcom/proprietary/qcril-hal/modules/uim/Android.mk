LOCAL_PATH := $(call my-dir)
QCRIL_DIR := ${LOCAL_PATH}/../..

include $(CLEAR_VARS)

LOCAL_CFLAGS               += -Wall -Werror $(qcril_cflags) -DFEATURE_QCRIL_UIM_QMI -DFEATURE_QCRIL_UIM_ISIM_REFRESH -DFEATURE_QCRIL_QMI_CAT
LOCAL_CXXFLAGS             += -std=c++14 $(qcril_cppflags)
LOCAL_CPPFLAGS             += -std=c++14 $(qcril_cppflags)  -DFEATURE_QCRIL_UIM_QMI -DFEATURE_QCRIL_UIM_ISIM_REFRESH -DFEATURE_QCRIL_QMI_CAT
LOCAL_LDFLAGS              += $(qcril_ldflags)
ifeq ($(QCRIL_BUILD_WITH_ASAN),true)
LOCAL_SANITIZE             += $(qcril_sanitize)
endif

LOCAL_SRC_FILES            += $(call all-cpp-files-under, src)
LOCAL_SRC_FILES            += $(call all-c-files-under, src)

LOCAL_MODULE               := qcrilUimModule
LOCAL_MODULE_OWNER         := qti
LOCAL_PROPRIETARY_MODULE   := true
LOCAL_MODULE_TAGS          := optional
LOCAL_HEADER_LIBRARIES     += libril-qc-hal-qmi-headers
LOCAL_SHARED_LIBRARIES     += liblog
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.uim@1.0
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.uim@1.1
LOCAL_SHARED_LIBRARIES     += vendor.qti.hardware.radio.uim_remote_server@1.0

include $(BUILD_STATIC_LIBRARY)
